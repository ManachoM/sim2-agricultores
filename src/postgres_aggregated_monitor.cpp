#include "../includes/postgres_aggregated_monitor.h"

#include "../includes/sim_config.h"

#include <bsp.h>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <unistd.h>
#include <vector>

// Funciones para enviar los logs entre procesadores
void send_logs(
    const std::map<int, std::map<std::string, double>> log, bsp_pid_t dest,
    int tag
);
std::map<int, std::map<std::string, double>> receive_logs();

std::string const insert_execution =
    ("INSERT INTO execution(execution_status_id, start_time) "
     "VALUES (1, to_timestamp($1, 'YYYY-MM-DDT0HH24:MI:SSZ')) "
     "RETURNING execution_id ");

std::string const insert_execution_params =
    "INSERT INTO execution_params (execution_id, param_name, param_value) "
    "VALUES ($1, $2, $3)";

std::string const update_execution_duration =
    "UPDATE execution SET execution_time = $1 WHERE execution_id = $2";

std::string const update_finished_execution =
    "UPDATE execution SET execution_status_id = 2 WHERE execution_id = $1";

std::string const update_failed_execution =
    "UPDATE execution SET execution_status_id = 3 WHERE execution_status_id != "
    "2 AND execution_id = $1";

std::string const insert_aggregated_product_result =
    "INSERT INTO aggregated_product_results (execution_id, process, time, "
    "product_id, value, proc_id) VALUES ($1, $2, $3, $4, $5, $6)";

std::string const insert_time_record =
    "INSERT INTO super_step_time_stat (execution_id, ss_number, proc_id, "
    "exec_time, "
    "sync_time) VALUES ($1, $2, $3, $4, $5)";

std::string const insert_event_record =
    "INSERT INTO super_step_event_stat (execution_id, ss_number, proc_id, "
    "agent_type, agent_process, amount) VALUES ($1, $2, $3, $4, $5, $6)";

PostgresAggregatedMonitor::PostgresAggregatedMonitor(
    std::string const &db_url, bool _debug, const int record_amount
)
    : Monitor("", _debug), _database_url(db_url)
{
  this->event_records.reserve(record_amount);
  this->time_records.reserve(record_amount);
  // Inicializamos el objeto con agregaciones de productos
  this->agg_logs["AGRICULTOR"] = std::map<int, std::map<std::string, double>>();
  this->agg_logs["FERIANTE"] = std::map<int, std::map<std::string, double>>();
  this->agg_logs["CONSUMIDOR"] = std::map<int, std::map<std::string, double>>();

  for (int i = 0; i < 21; ++i)
  {
    this->agg_logs["AGRICULTOR"][0][std::to_string(i)] = 0;
    this->agg_logs["FERIANTE"][0][std::to_string(i)] = 0;
    this->agg_logs["CONSUMIDOR"][0][std::to_string(i)] = 0;
  }

  json conf = SimConfig::get_instance()->get_config();

  if (this->_database_url == "")
    this->_database_url = conf["DB_URL"].get<std::string>();
  printf("%s\n", this->_database_url.c_str());
  // Set tag size to 0 for compatibility with initialize_agents
  // This is important because ParallelSimulation wasn't designed to handle tags
  int tag_size = 0;
  bsp_set_tagsize(&tag_size);

  bsp_sync();
  if (bsp_pid() == 0)
  {
    // Generamos la conexión
    pqxx::connection conn(this->_database_url);

    // Preparamos las queries
    conn.prepare("insert_execution", insert_execution);
    conn.prepare("insert_exec_param", insert_execution_params);

    // Registramos la ejecución y sus parámetros
    pqxx::work t{conn};
    std::string now = get_current_timestamp().c_str();

    this->execution_id =
        t.exec_prepared1("insert_execution", get_current_timestamp().c_str())[0]
            .as<int>();
    for (auto it = conf.begin(); it != conf.end(); ++it)
    {
      if (it.key() == "DB_URL")
        continue;

      auto value = it.value();
      std::string str_val;
      if (value.is_boolean())
      {
        str_val = value.get<bool>() ? "true" : "false";
      }
      else if (value.is_number_integer())
      {
        str_val = std::to_string(value.get<int>());
      }
      else if (value.is_number_float())
      {
        str_val = std::to_string(value.get<double>());
      }
      else if (value.is_string())
      {
        str_val = value.get<std::string>();
      }
      else
      {
        throw std::logic_error(
            "Tipo de variable de configuración no soportado.\n"
        );
      }

      t.exec_prepared0(
          "insert_exec_param", this->execution_id, it.key().c_str(),
          str_val.c_str()
      );
    }
    
    // Guardamos el nombre del archivo de config
    t.exec_prepared0(
        "insert_exec_param", this->execution_id, "sim_config_file",
        SimConfig::get_instance()->get_config_file_path()
    );
    
    // Save number of processors - make explicit call to bsp_nprocs()
    int nprocs = bsp_nprocs();
    t.exec_prepared0(
        "insert_exec_param", this->execution_id, "num_processors",
        std::to_string(nprocs).c_str()
    );
    
    // Save BSP window size from global variable
    extern double WINDOW_SIZE; // Defined in main.h
    t.exec_prepared0(
        "insert_exec_param", this->execution_id, "window_size",
        std::to_string(WINDOW_SIZE).c_str()
    );

    // Commit y cerrar la conexión
    t.commit();

    // ahora mandamos el execution_id a todos los otros procs
    int value = this->execution_id;
    int tag = 0; // Tag for execution_id message
    for (int i = 1; i < nprocs; ++i)
    {
      bsp_send(i, &tag, &value, sizeof(int));
    }
  }
  bsp_sync();
  if (bsp_pid() != 0)
  {
    // Each non-root process checks its message queue.
    bsp_nprocs_t nmsgs;
    bsp_size_t totalPayload;
    bsp_qsize(&nmsgs, &totalPayload);
    if (nmsgs > 0 && totalPayload == sizeof(int))
    {
      int received_value = 0;
      bsp_move(&received_value, sizeof(int));
      this->execution_id = received_value;
    }
    else
    {
      printf("Process %d: No message received.\n", bsp_pid());
    }
  }
  printf("Execution id %d\n", this->execution_id);
  bsp_sync();
}

void PostgresAggregatedMonitor::add_event_record(SSEventRecord e)
{
  this->event_records.push_back(e);
}

void PostgresAggregatedMonitor::add_time_record(SSTimeRecord e)
{
  this->time_records.push_back(e);
}

void PostgresAggregatedMonitor::write_duration(double time)
{
  if (bsp_pid() != 0)
    return;
  printf("on write_duration %s\n", this->_database_url.c_str());
  pqxx::connection conn(this->_database_url);
  conn.prepare("update_execution_duration", update_execution_duration);
  conn.prepare("update_finished_execution", update_finished_execution);

  pqxx::work t{conn};

  t.exec_prepared0("update_execution_duration", time, this->execution_id);
  t.exec_prepared0("update_finished_execution", this->execution_id);

  // Commiteamos y la conexión se pierde
  t.commit();
}

void PostgresAggregatedMonitor::write_log(json &log)
{
  // Obtenemos el mes y el año al que pertenece el nuevo evento
  std::string year = std::to_string(int(log["time"].get<double>() / 8'760));
  std::string month = std::to_string(int(log["time"].get<double>() / 720));

  int month_n = int(log["time"].get<double>() / 720);

  // Si el mes es distinto al último recibido, se inicializa una nueva entrada
  // para cada producto

  if (month != std::to_string(this->last_recorded_month))
  {
    for (int i = 0; i < 21; ++i)
    {
      this->agg_logs["AGRICULTOR"][month_n][std::to_string(i)] = 0;
      this->agg_logs["FERIANTE"][month_n][std::to_string(i)] = 0;
      this->agg_logs["CONSUMIDOR"][month_n][std::to_string(i)] = 0;
    }
  }

  if (this->debug_flag)
    std::cout << log.dump() << std::endl;
  // Agregamos los resultados según el tipo de agente
  if (log["agent_type"].get<std::string>() == "AGRICULTOR" &&
      log["agent_process"].get<std::string>() == "COSECHA")
  {
    //    printf("LOGGEANDO AGRICULTOR\n");
    double cantidad = log["cantidad_cosechada"].get<double>();
    // printf("aki voi\n");
    // printf("[LOG] - [AGRICULTOR] - [COSECHA] - ID Prod: %d\tCantidad
    // cosechada: %f\t Cantidad acumulada del mes: %f\n ",
    // log["producto_cosechado"].get<int>(), cantidad,
    // this->agg_logs.find("AGRICULTOR")->second.find(month_n)->second.find(std::to_string(log["producto_cosechado"].get<int>()))->second);
    this->agg_logs["AGRICULTOR"][month_n]
                  [std::to_string(log["producto_cosechado"].get<int>())] +=
        cantidad;
    // this->aggregated_logs["AGRICULTOR"][year][month][std::to_string(log["producto_cosechado"].get<int>())]
    // = +log["cantidad_cosechada"].get<double>();
  }
  else if (log["agent_type"].get<std::string>() == "FERIANTE" &&
           log["agent_process"].get<std::string>() ==
               "PROCESAR_COMPRA_AGRICULTOR")
  {
    // std::cout << log.dump() << "\n";
    // printf("LOGGEANDO FERIANTE\n");
    // auto compras = log["compras"];
    // Si el id de agricultor es -1, no se concretó una compra
    try
    {

      if (log["id_agricultor"].get<int>() == -1)
      {
        return;
      }
    }
    catch (const nlohmann::detail::type_error &e)
    {
      std::cerr << log.dump(4) << '\n';
      std::cerr << e.what() << '\n';
      return;
    }
    // printf(
    //     "Registrando compra a agricultor ID Agro: %d ID Prod: %d Amount:
    //     %lf\n", log["id_agricultor"].get<int>(),
    //     log["target_product"].get<int>(), log["target_amount"].get<double>()
    //);
    std::string target_prod = std::to_string(log["target_product"].get<int>());
    double cantidad = log["target_amount"].get<double>();
    this->agg_logs["FERIANTE"][month_n][target_prod] += cantidad;
  }
  else if (log["agent_type"].get<std::string>() == "CONSUMIDOR" &&
           log["agent_process"].get<std::string>() ==
               "PROCESAR_COMPRA_FERIANTE")
  {
    // auto compras = log["compras"];
    // Si el id de feriante es -1, no se concretó una compra
    try
    {
      if (log["feriante_id"].get<int>() == -1)
        return;
    }
    catch (const nlohmann::detail::type_error &e)
    {
      std::cerr << log.dump(4) << '\n';
      std::cerr << e.what() << '\n';
    }

    std::string target_prod = std::to_string(log["target_product"].get<int>());
    this->agg_logs["CONSUMIDOR"][month_n][target_prod] +=
        log["target_amount"].get<double>();

    // this->aggregated_logs["CONSUMIDOR"][year][month][target_prod] =
    // +it.value()["target_amount"].get<double>();
  }
  else if (log["agent_type"].get<std::string>() == "CONSUMIDOR" &&
           log["agent_process"].get<std::string>() == "COMPRA_FERIANTE")
  {
    // printf("Procesando nuevo evento del consumidor\n");
    // Obtenemos la lista de compras
    std::vector<json> compras = log["compras"].get<std::vector<json>>();

    for (const json &compra : compras)
    {
      std::string target_prod =
          std::to_string(compra["target_product"].get<int>());
      this->agg_logs["CONSUMIDOR"][month_n][target_prod] +=
          compra["target_amount"].get<double>();
    }
  }
  else if (log["agent_type"].get<std::string>() == "FERIANTE" &&
           log["agent_process"].get<std::string>() == "COMPRA_MAYORISTA")
  {
    std::vector<json> compras = log["compras"].get<std::vector<json>>();
    for (const json &compra : compras)
    {
      std::string target_prod =
          std::to_string(compra["target_product"].get<int>());
      double cantidad = compra["target_amount"].get<double>();
      this->agg_logs["FERIANTE"][month_n][target_prod] += cantidad;
    }
  }
  this->last_recorded_month = short(log["time"].get<double>() / 720);
}

void PostgresAggregatedMonitor::write_results()
{
  // Definimos constantes para los tipos de proceso para evitar inconsistencias
  const std::string PROCESS_FERIANTE = "COMPRA DE FERIANTE A AGRICULTOR";
  const std::string PROCESS_CONSUMIDOR = "COMPRA DE CONSUMIDOR";
  const std::string PROCESS_AGRICULTOR = "COSECHA AGRICULTOR";

  int pid = bsp_pid();
  int nprocs = bsp_nprocs();
  
  // Print minimal diagnostic info
  if (pid == 0) {
    printf("\nStarting write_results() on processor %d\n", pid);
  }
  
  // Create temporary file with processor-specific results
  std::string temp_file = "/tmp/sim_results_proc_" + std::to_string(pid) + ".dat";
  std::ofstream out_file(temp_file);
  
  if (!out_file) {
    std::cerr << "Error creating temporary file on processor " << pid << std::endl;
    return;
  }
  
  // Write FERIANTE data - only non-zero values
  for (auto const &[month, cantidad_por_prod] : this->agg_logs["FERIANTE"]) {
    for (auto const &[prod_id, cantidad] : cantidad_por_prod) {
      if (cantidad > 0) {
        out_file << "FERIANTE|" << month << "|" << prod_id << "|" << cantidad << "|" << pid << "\n";
      }
    }
  }
  
  // Write CONSUMIDOR data - only non-zero values
  for (auto const &[month, cantidad_por_prod] : this->agg_logs["CONSUMIDOR"]) {
    for (auto const &[prod_id, cantidad] : cantidad_por_prod) {
      if (cantidad > 0) {
        out_file << "CONSUMIDOR|" << month << "|" << prod_id << "|" << cantidad << "|" << pid << "\n";
      }
    }
  }
  
  // Write AGRICULTOR data - only non-zero values
  for (auto const &[month, cantidad_por_prod] : this->agg_logs["AGRICULTOR"]) {
    for (auto const &[prod_id, cantidad] : cantidad_por_prod) {
      if (cantidad > 0) {
        out_file << "AGRICULTOR|" << month << "|" << prod_id << "|" << cantidad << "|" << pid << "\n";
      }
    }
  }
  
  // Write time records - more efficient format
  for (auto const &event : this->time_records) {
    out_file << "TIME|" << event.ss_number << "|" << event.proc_id << "|" 
             << event.exec_time << "|" << event.sync_time << "\n";
  }
  
  // Clear time records to free memory
  this->time_records.clear();
  
  // Write event records - simpler approach
  for (auto const &event : this->event_records) {
    for (auto const &[event_type, count] : event.event_type_count) {
      if (count > 0) {
        out_file << "EVENT|" << event.ss_number << "|" << event.proc_id << "|EVENT_TYPE|" 
                 << event_type << "|" << count << "\n";
      }
    }
    
    for (auto const &[agent_type, count] : event.agent_type_count) {
      if (count > 0) {
        out_file << "EVENT|" << event.ss_number << "|" << event.proc_id << "|AGENT_TYPE|" 
                 << agent_type << "|" << count << "\n";
      }
    }
  }
  
  // Clear event records to free memory
  this->event_records.clear();
  
  out_file.close();
  
  // Synchronize to ensure all processors have written their files
  bsp_sync();
  
  // Only processor 0 handles database operations
  if (pid == 0) {
    printf("Processor 0: Processing data from temporary files\n");
    try {
      pqxx::connection conn(this->_database_url);
      conn.prepare("insert_aggregated_product_result", insert_aggregated_product_result);
      conn.prepare("insert_time_record", insert_time_record);
      conn.prepare("insert_event_record", insert_event_record);
      
      // Process files directly without loading everything into memory
      for (int proc = 0; proc < nprocs; proc++) {
        std::string file_path = "/tmp/sim_results_proc_" + std::to_string(proc) + ".dat";
        std::ifstream in_file(file_path);
        
        if (!in_file) {
          std::cerr << "Warning: Could not open temporary file from processor " << proc << std::endl;
          continue;
        }
        
        printf("Processing data from processor %d\n", proc);
        std::string line;
        int line_count = 0;
        const int BATCH_SIZE = 1000;
        std::vector<std::string> agent_batch;
        std::vector<std::string> time_batch;
        std::vector<std::string> event_batch;
        
        auto process_batches = [&]() {
          if (!agent_batch.empty() || !time_batch.empty() || !event_batch.empty()) {
            // Create a new transaction for each batch
            pqxx::work t{conn};
            
            // Process agent data batch
            for (const auto& data : agent_batch) {
              std::istringstream iss(data);
              std::string agent_type, month_str, prod_id, quantity, source_proc;
              
              std::getline(iss, agent_type, '|');
              std::getline(iss, month_str, '|');
              std::getline(iss, prod_id, '|');
              std::getline(iss, quantity, '|');
              std::getline(iss, source_proc, '|');
              
              int month = std::stoi(month_str);
              double cantidad = std::stod(quantity);
              int proc_id = std::stoi(source_proc);
              
              std::string process_type;
              if (agent_type == "FERIANTE") process_type = PROCESS_FERIANTE;
              else if (agent_type == "CONSUMIDOR") process_type = PROCESS_CONSUMIDOR;
              else process_type = PROCESS_AGRICULTOR;
              
              // Insert directly with proc_id
              t.exec_prepared0(
                  "insert_aggregated_product_result", this->execution_id,
                  process_type, month, prod_id, cantidad, proc_id
              );
            }
            
            // Process time record batch
            for (const auto& data : time_batch) {
              std::istringstream iss(data);
              std::string ss_number, proc_id, exec_time, sync_time;
              
              std::getline(iss, ss_number, '|');
              std::getline(iss, proc_id, '|');
              std::getline(iss, exec_time, '|');
              std::getline(iss, sync_time, '|');
              
              t.exec_prepared0(
                  "insert_time_record", this->execution_id, std::stoi(ss_number),
                  std::stoi(proc_id), std::stod(exec_time), std::stod(sync_time)
              );
            }
            
            // Process event record batch
            for (const auto& data : event_batch) {
              std::istringstream iss(data);
              std::string ss_number, proc_id, type_cat, type_name, count;
              
              std::getline(iss, ss_number, '|');
              std::getline(iss, proc_id, '|');
              std::getline(iss, type_cat, '|');
              std::getline(iss, type_name, '|');
              std::getline(iss, count, '|');
              
              std::string agent_type = "";
              std::string event_type = "";
              
              if (type_cat == "AGENT_TYPE") {
                  agent_type = type_name;
              } else {
                  event_type = type_name;
              }
              
              t.exec_prepared0(
                  "insert_event_record", this->execution_id, std::stoi(ss_number),
                  std::stoi(proc_id), agent_type, event_type, std::stoi(count)
              );
            }
            
            // Commit this batch
            t.commit();
            
            // Clear batches
            agent_batch.clear();
            time_batch.clear();
            event_batch.clear();
          }
        };
        
        while (std::getline(in_file, line)) {
          if (line.empty() || line[0] == '#') continue;
          
          size_t pos = line.find('|');
          if (pos == std::string::npos) continue;
          
          std::string record_type = line.substr(0, pos);
          std::string rest = line.substr(pos + 1);
          
          if (record_type == "FERIANTE" || record_type == "CONSUMIDOR" || record_type == "AGRICULTOR") {
            agent_batch.push_back(record_type + "|" + rest);
            line_count++;
          }
          else if (record_type == "TIME") {
            time_batch.push_back(rest);
            line_count++;
          }
          else if (record_type == "EVENT") {
            event_batch.push_back(rest);
            line_count++;
          }
          
          // Process in batches to avoid memory buildup
          if (line_count >= BATCH_SIZE) {
            process_batches();
            line_count = 0;
          }
        }
        
        // Process any remaining records
        process_batches();
        
        in_file.close();
        
        // Remove temporary file after processing
        std::remove(file_path.c_str());
      }
      
      printf("Processor 0: Database write complete\n");
    }
    catch (const std::exception &e) {
      std::cerr << "Database error: " << e.what() << std::endl;
    }
  }
  
  bsp_sync();
}

void PostgresAggregatedMonitor::write_params(
    const std::string &key, const std::string &value
)
{
  // Only processor 0 should write to the database
  if (bsp_pid() != 0) {
    return;
  }
  
  pqxx::connection conn(this->_database_url);
  conn.prepare("insert_exec_param", insert_execution_params);

  pqxx::work t{conn};

  t.exec_prepared0(
      "insert_exec_param", this->execution_id, key.c_str(), value.c_str()
  );

  t.commit();
}

PostgresAggregatedMonitor::~PostgresAggregatedMonitor()
{
  pqxx::connection conn(this->_database_url);

  // Marcamos como fallida si la ejecución no ha terminado
  // para cuando muere el monitor: algo anda mal.
  conn.prepare("update_failed_execution", update_failed_execution);

  pqxx::work t{conn};

  t.exec_prepared0("update_failed_execution", this->execution_id);
  t.commit();

  // for (auto const &[first, second] : this->agg_logs)
  // {
  //     std::cout << first << "\n";
  //     for (auto const &[month, value] : second)
  //     {
  //         std::cout << "MES: " << month << "\t";
  //         for (auto const &[key, val] : value)
  //         {
  //             std::cout << "LLAVE: " << key << " VALOR: " << val << "\t";
  //         }
  //         std::cout << "\n";
  //     }
  //     std::cout << "\n\n";
  // }
}

std::string PostgresAggregatedMonitor::get_current_timestamp()
{
  // Get current time
  auto now = std::chrono::system_clock::now();

  // Convert to time_t (seconds since epoch)
  std::time_t time = std::chrono::system_clock::to_time_t(now);

  // Convert to tm struct (broken down time)
  std::tm tm = *std::gmtime(&time);

  // Format the time as ISO 8601
  char buffer[25]; // Enough space for ISO 8601 format
  std::strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ", &tm);

  return buffer;
}

// This function is now deprecated - use temporary files instead
void send_logs(
    const std::map<int, std::map<std::string, double>> log, bsp_pid_t dest,
    int tag
)
{
  // Serializamos el mensaje
  std::ostringstream oss;
  for (const auto &outer : log)
  {
    oss << outer.first << "|";
    for (const auto &inner : outer.second)
    {
      oss << inner.first << ":" << inner.second << ",";
    }
    oss << ";";
  }
  std::string msg = oss.str();

  // Enviamos el mensaje usando bsp_send con el tag proporcionado
  bsp_send(dest, &tag, msg.data(), msg.size());
}

// This function is now deprecated - use temporary files instead
std::map<int, std::map<std::string, double>> receive_logs()
{
  bsp_size_t numMessages, totalBytes;
  bsp_qsize(&numMessages, &totalBytes);

  if (numMessages == 0)
  {
    return {}; // No messages received
  }
  
  bsp_size_t status, tag;
  bsp_get_tag(&status, &tag); // Get the payload size and tag

  if (status == 0) {
    return {}; // Empty message
  }

  std::vector<char> buffer(status);
  bsp_move(buffer.data(), status);
  
  // Deserialización
  std::map<int, std::map<std::string, double>> data;
  std::string bufferStr(buffer.begin(), buffer.end());
  std::istringstream iss(bufferStr);
  std::string outer_token;
  while (std::getline(iss, outer_token, ';'))
  {
    std::istringstream outerStream(outer_token);
    std::string keyPart, innerData;
    if (std::getline(outerStream, keyPart, '|') &&
        std::getline(outerStream, innerData))
    {
      int outerKey = std::stoi(keyPart);
      std::map<std::string, double> innerMap;

      std::istringstream innerStream(innerData);
      std::string innerToken;
      while (std::getline(innerStream, innerToken, ','))
      {
        size_t pos = innerToken.find(":");
        if (pos != std::string::npos)
        {
          std::string innerKey = innerToken.substr(0, pos);
          double innerValue = std::stod(innerToken.substr(pos + 1));
          innerMap[innerKey] = innerValue;
        }
      }
      data[outerKey] = innerMap;
    }
  }
  return data;
}

void PostgresAggregatedMonitor::gather_records()
{
  // This method is now deprecated - use the file-based approach in write_results instead
  printf("Note: gather_records is deprecated and now redirects to write_results\n");
  
  // Just call write_results which handles all gathering now
  write_results();
}
