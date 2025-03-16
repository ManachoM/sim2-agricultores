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
    "product_id, value) VALUES ($1, $2, $3, $4, $5)";

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

    // Commit y cerrar la conexión
    t.commit();

    // ahora mandamos el execution_id a todos los otros procs
    int nprocs = bsp_nprocs();
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
  
  // Create temporary file with processor-specific results
  std::string temp_file = "/tmp/sim_results_proc_" + std::to_string(pid) + ".dat";
  std::ofstream out_file(temp_file, std::ios::binary);
  
  if (!out_file) {
    std::cerr << "Error creating temporary file on processor " << pid << std::endl;
    return;
  }
  
  // Add a header line with processor info for debugging
  out_file << "# Results from processor " << pid << " of " << nprocs << "\n";
  
  // Write all data types to the file in a structured format
  // Format: AGENT_TYPE|MONTH|PROD_ID|QUANTITY
  
  // Write FERIANTE data
  for (auto const &[month, cantidad_por_prod] : this->agg_logs["FERIANTE"]) {
    for (auto const &[prod_id, cantidad] : cantidad_por_prod) {
      if (cantidad > 0) { // Only write non-zero values
        out_file << "FERIANTE|" << month << "|" << prod_id << "|" << cantidad << "\n";
      }
    }
  }
  
  // Write CONSUMIDOR data
  for (auto const &[month, cantidad_por_prod] : this->agg_logs["CONSUMIDOR"]) {
    for (auto const &[prod_id, cantidad] : cantidad_por_prod) {
      if (cantidad > 0) { // Only write non-zero values
        out_file << "CONSUMIDOR|" << month << "|" << prod_id << "|" << cantidad << "\n";
      }
    }
  }
  
  // Write AGRICULTOR data
  for (auto const &[month, cantidad_por_prod] : this->agg_logs["AGRICULTOR"]) {
    for (auto const &[prod_id, cantidad] : cantidad_por_prod) {
      if (cantidad > 0) { // Only write non-zero values
        out_file << "AGRICULTOR|" << month << "|" << prod_id << "|" << cantidad << "\n";
      }
    }
  }
  
  // Write time records in the format TIME|SS_NUMBER|PROC_ID|EXEC_TIME|SYNC_TIME
  for (auto const &event : this->time_records) {
    out_file << "TIME|" << event.ss_number << "|" << event.proc_id << "|" 
             << event.exec_time << "|" << event.sync_time << "\n";
  }
  
  // Write event records in the format EVENT|SS_NUMBER|PROC_ID|AGENT_TYPE|EVENT_TYPE|COUNT
  for (auto const &event : this->event_records) {
    // Write event type counts
    for (auto const &[event_type, count] : event.event_type_count) {
      out_file << "EVENT|" << event.ss_number << "|" << event.proc_id << "||" 
               << event_type << "|" << count << "\n";
    }
    
    // Write agent type counts
    for (auto const &[agent_type, count] : event.agent_type_count) {
      out_file << "EVENT|" << event.ss_number << "|" << event.proc_id << "|" 
               << agent_type << "||" << count << "\n";
    }
  }
  
  out_file.close();
  
  // Synchronize to ensure all processors have written their files
  bsp_sync();
  
  // Only processor 0 handles database operations
  if (pid == 0) {
    printf("Processor 0: Collecting data from all temporary files\n");
    
    // Maps to collect all data efficiently
    std::map<std::string, std::map<int, std::map<std::string, double>>> all_agent_data;
    std::vector<SSTimeRecord> all_time_records;
    std::vector<std::pair<int, std::pair<std::string, std::string>>> all_event_records;
    
    // Initialize agent data structure
    all_agent_data["FERIANTE"] = std::map<int, std::map<std::string, double>>();
    all_agent_data["CONSUMIDOR"] = std::map<int, std::map<std::string, double>>();
    all_agent_data["AGRICULTOR"] = std::map<int, std::map<std::string, double>>();
    
    // Process files from all processors
    for (int proc = 0; proc < nprocs; proc++) {
      std::string file_path = "/tmp/sim_results_proc_" + std::to_string(proc) + ".dat";
      std::ifstream in_file(file_path);
      
      if (!in_file) {
        std::cerr << "Warning: Could not open temporary file from processor " << proc << std::endl;
        continue;
      }
      
      std::string line;
      while (std::getline(in_file, line)) {
        std::istringstream iss(line);
        std::string record_type, field1, field2, field3, field4, field5;
        
        // Parse the first part to determine record type
        size_t pos = line.find('|');
        if (pos == std::string::npos) continue;
        
        record_type = line.substr(0, pos);
        std::string rest = line.substr(pos + 1);
        
        if (record_type == "FERIANTE" || record_type == "CONSUMIDOR" || record_type == "AGRICULTOR") {
          // Parse agent data: AGENT_TYPE|MONTH|PROD_ID|QUANTITY
          std::istringstream fields(rest);
          std::getline(fields, field1, '|'); // month
          std::getline(fields, field2, '|'); // prod_id
          std::getline(fields, field3, '|'); // quantity
          
          int month = std::stoi(field1);
          std::string prod_id = field2;
          double cantidad = std::stod(field3);
          
          // Add to the aggregated data structure
          all_agent_data[record_type][month][prod_id] += cantidad;
        }
        else if (record_type == "TIME") {
          // Parse time record: TIME|SS_NUMBER|PROC_ID|EXEC_TIME|SYNC_TIME
          std::istringstream fields(rest);
          std::getline(fields, field1, '|'); // ss_number
          std::getline(fields, field2, '|'); // proc_id
          std::getline(fields, field3, '|'); // exec_time
          std::getline(fields, field4, '|'); // sync_time
          
          SSTimeRecord record;
          record.ss_number = std::stoi(field1);
          record.proc_id = std::stoi(field2);
          record.exec_time = std::stod(field3);
          record.sync_time = std::stod(field4);
          
          all_time_records.push_back(record);
        }
        else if (record_type == "EVENT") {
          // Parse event record: EVENT|SS_NUMBER|PROC_ID|AGENT_TYPE|EVENT_TYPE|COUNT
          std::istringstream fields(rest);
          std::getline(fields, field1, '|'); // ss_number
          std::getline(fields, field2, '|'); // proc_id
          std::getline(fields, field3, '|'); // agent_type
          std::getline(fields, field4, '|'); // event_type
          std::getline(fields, field5, '|'); // count
          
          int ss_number = std::stoi(field1);
          int count = std::stoi(field5);
          
          // Store ss_number, agent_type, event_type, count
          all_event_records.push_back({ss_number, {field3, field4}});
        }
      }
      
      in_file.close();
      // Remove temporary file after processing
      std::remove(file_path.c_str());
    }
    
    // Now write everything to the database in a single transaction
    printf("Processor 0: Writing all collected data to database\n");
    try {
      pqxx::connection conn(this->_database_url);
      
      // Prepare all statements once
      conn.prepare("insert_aggregated_product_result", insert_aggregated_product_result);
      conn.prepare("insert_time_record", insert_time_record);
      conn.prepare("insert_event_record", insert_event_record);
      
      // Single transaction for all data
      pqxx::work t{conn};
      
      // Write agent data
      for (auto const &[month, cantidad_por_prod] : all_agent_data["FERIANTE"]) {
        for (auto const &[prod_id, cantidad] : cantidad_por_prod) {
          t.exec_prepared0(
              "insert_aggregated_product_result", this->execution_id,
              PROCESS_FERIANTE, month, prod_id, cantidad
          );
        }
      }
      
      for (auto const &[month, cantidad_por_prod] : all_agent_data["CONSUMIDOR"]) {
        for (auto const &[prod_id, cantidad] : cantidad_por_prod) {
          t.exec_prepared0(
              "insert_aggregated_product_result", this->execution_id,
              PROCESS_CONSUMIDOR, month, prod_id, cantidad
          );
        }
      }
      
      for (auto const &[month, cantidad_por_prod] : all_agent_data["AGRICULTOR"]) {
        for (auto const &[prod_id, cantidad] : cantidad_por_prod) {
          t.exec_prepared0(
              "insert_aggregated_product_result", this->execution_id,
              PROCESS_AGRICULTOR, month, prod_id, cantidad
          );
        }
      }
      
      // Write time records
      for (auto const &event : all_time_records) {
        t.exec_prepared0(
            "insert_time_record", this->execution_id, event.ss_number,
            event.proc_id, event.exec_time, event.sync_time
        );
      }
      
      // Write event records - group by ss_number for batching
      std::map<int, std::map<std::string, int>> event_type_counts;
      std::map<int, std::map<std::string, int>> agent_type_counts;
      
      for (auto const &[ss_number, type_pair] : all_event_records) {
        const auto &[agent_type, event_type] = type_pair;
        
        if (!agent_type.empty() && event_type.empty()) {
          // This is an agent type count
          agent_type_counts[ss_number][agent_type]++;
        }
        else if (agent_type.empty() && !event_type.empty()) {
          // This is an event type count
          event_type_counts[ss_number][event_type]++;
        }
      }
      
      // Insert agent type counts
      for (auto const &[ss_number, counts] : agent_type_counts) {
        for (auto const &[agent_type, count] : counts) {
          t.exec_prepared0(
              "insert_event_record", this->execution_id, ss_number,
              0, agent_type, "", count
          );
        }
      }
      
      // Insert event type counts
      for (auto const &[ss_number, counts] : event_type_counts) {
        for (auto const &[event_type, count] : counts) {
          t.exec_prepared0(
              "insert_event_record", this->execution_id, ss_number,
              0, "", event_type, count
          );
        }
      }
      
      // Commit all data in a single transaction
      t.commit();
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
