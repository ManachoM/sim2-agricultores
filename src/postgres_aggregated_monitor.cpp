#include "../includes/postgres_aggregated_monitor.h"

#include "../includes/sim_config.h"

#include <bsp.h>
#include <cstdio>
#include <ctime>
#include <map>
#include <sstream>
#include <string>
#include <unistd.h>

// Funciones para enviar los logs entre procesadores
void send_logs(
    const std::map<int, std::map<std::string, double>> log, bsp_pid_t dest,
    int tag = 0
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
  // Set the tag size to sizeof(int) to allow for message type identification
  int tag_size = sizeof(int);
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

  // Si somos el procesador principal, escribimos nuestros resultados primero
  if (bsp_pid() == 0)
  {
    // Preparamos la conexión y el statement
    pqxx::connection conn(this->_database_url);
    conn.prepare(
        "insert_aggregated_product_result", insert_aggregated_product_result
    );
    pqxx::work t{conn};
    // Primero, se escriben los logs locales
    // Pasaremos por los tres primeros niveles de anidado para poder ser
    // precisos con el mensaje en DB
    for (auto const &[month, cantidad_por_prod] : this->agg_logs["FERIANTE"])
    {
      for (auto const &[prod_id, cantidad] : cantidad_por_prod)
      {
        t.exec_prepared0(
            "insert_aggregated_product_result", this->execution_id,
            PROCESS_FERIANTE, month, prod_id, cantidad
        );
      }
    }

    for (auto const &[month, cantidad_por_prod] : this->agg_logs["CONSUMIDOR"])
    {
      for (auto const &[prod_id, cantidad] : cantidad_por_prod)
      {
        t.exec_prepared0(
            "insert_aggregated_product_result", this->execution_id,
            PROCESS_CONSUMIDOR, month, prod_id, cantidad
        );
      }
    }

    for (auto const &[month, cantidad_por_prod] : this->agg_logs["AGRICULTOR"])
    {
      for (auto const &[prod_id, cantidad] : cantidad_por_prod)
      {
        t.exec_prepared0(
            "insert_aggregated_product_result", this->execution_id,
            PROCESS_AGRICULTOR, month, prod_id, cantidad
        );
      }
    }
    t.commit();
  }

  // Estructura para almacenar datos a enviar a proceso 0
  struct AgentLogData
  {
    std::string agent_type;
    std::map<int, std::map<std::string, double>> log_data;
  };

  // Recolectamos todos los datos de todos los procesadores
  std::vector<AgentLogData> logs_to_send;

  // Si no somos el procesador 0, enviamos todos nuestros logs
  if (bsp_pid() != 0)
  {
    // Agregamos cada tipo de agente a nuestra colección para enviar
    if (!this->agg_logs["FERIANTE"].empty())
    {
      logs_to_send.push_back({"FERIANTE", this->agg_logs["FERIANTE"]});
    }

    if (!this->agg_logs["CONSUMIDOR"].empty())
    {
      logs_to_send.push_back({"CONSUMIDOR", this->agg_logs["CONSUMIDOR"]});
    }

    if (!this->agg_logs["AGRICULTOR"].empty())
    {
      logs_to_send.push_back({"AGRICULTOR", this->agg_logs["AGRICULTOR"]});
    }

    // Enviamos todos los logs en un solo paso, con una etiqueta con el tipo de
    // agente
    for (const auto &log_data : logs_to_send)
    {
      // Creamos una etiqueta con el tipo de agente (FERIANTE, CONSUMIDOR,
      // AGRICULTOR)
      int agent_type_tag = 0;
      if (log_data.agent_type == "FERIANTE")
        agent_type_tag = 1;
      else if (log_data.agent_type == "CONSUMIDOR")
        agent_type_tag = 2;
      else if (log_data.agent_type == "AGRICULTOR")
        agent_type_tag = 3;

      // Enviamos el log con la etiqueta correspondiente
      send_logs(log_data.log_data, 0, agent_type_tag);
    }
  }

  bsp_sync();

  // Recibimos los logs, los parseamos y los guardamos en la DB
  if (bsp_pid() == 0)
  {
    // Preparamos la conexión una sola vez para toda la operación
    pqxx::connection conn(this->_database_url);
    conn.prepare(
        "insert_aggregated_product_result", insert_aggregated_product_result
    );

    // Recibimos los mensajes
    bsp_size_t num_messages, total_bytes;
    bsp_qsize(&num_messages, &total_bytes);

    // Preparamos una sola transacción para toda la operación
    pqxx::work t{conn};

    for (bsp_size_t i = 0; i < num_messages; ++i)
    {
      bsp_size_t status, tag;
      bsp_get_tag(&status, &tag);

      // Determinamos el tipo de proceso basado en la etiqueta recibida
      std::string process_type;
      if (tag == 1)
        process_type = PROCESS_FERIANTE;
      else if (tag == 2)
        process_type = PROCESS_CONSUMIDOR;
      else if (tag == 3)
        process_type = PROCESS_AGRICULTOR;
      else
        continue; // Ignoramos mensajes con etiquetas desconocidas

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

      // Ahora que data tiene el log recibido, lo guardamos con el tipo de
      // proceso correcto
      for (auto const &[month, cantidad_por_prod] : data)
      {
        for (auto const &[prod_id, cantidad] : cantidad_por_prod)
        {
          t.exec_prepared0(
              "insert_aggregated_product_result", this->execution_id,
              process_type, month, prod_id, cantidad
          );
        }
      }
    }

    // Commit de todos los inserts en una sola transacción
    t.commit();
  }

  bsp_sync();

  // this->gather_records();

  /// if (bsp_pid() == 0)
  //{

  // Ahora guardamos los registros de cada SS
  // Esto genera una conexión por LP
  // OJO!!!!👀
  pqxx::connection conn(this->_database_url);
  conn.prepare("insert_event_record", insert_event_record);

  conn.prepare("insert_time_record", insert_time_record);

  // Primero por los de tiempo, que son los más sencillos
  pqxx::work t{conn};
  for (auto event : this->time_records)
  {
    t.exec_prepared0(
        "insert_time_record", this->execution_id, event.ss_number,
        event.proc_id, event.exec_time, event.sync_time
    );
  }
  t.commit();

  // Ahora vamos con los de evento
  for (auto event : this->event_records)
  {
    for (auto event_type_record : event.event_type_count)
    {
      t.exec_prepared0(
          "insert_event_record", this->execution_id, event.ss_number,
          event.proc_id, "", event_type_record.first, event_type_record.second
      );
    }

    for (auto agent_type_record : event.agent_type_count)
    {
      t.exec_prepared0(
          "insert_event_record", this->execution_id, event.ss_number,
          event.proc_id, agent_type_record.first, "", agent_type_record.second
      );
    }

    t.commit();
  }
  //}
  bsp_sync();
}

void PostgresAggregatedMonitor::write_params(
    const std::string &key, const std::string &value
)
{
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

void send_logs(
    const std::map<int, std::map<std::string, double>> log, bsp_pid_t dest,
    int tag = 0
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

std::map<int, std::map<std::string, double>> receive_logs()
{
  bsp_size_t numMessages, totalBytes;
  bsp_qsize(&numMessages, &totalBytes);

  if (numMessages == 0)
  {
    return {}; // No messages received
  }

  bsp_size_t status, tag;
  bsp_get_tag(&status, &tag); // Get the payload size

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
  // Tag size already set in constructor to sizeof(int)
  bsp_sync();

  int pid = bsp_pid();
  printf("entrando a gather_records\n");
  // Non-root processors send their records to process 0.
  if (pid != 0)
  {
    // --- Send time records ---
    int timeCount = time_records.size();
    // Tag '1' identifies time record messages.
    int tag = 1;
    if (timeCount > 0)
      bsp_send(0, &tag, time_records.data(), timeCount * sizeof(SSTimeRecord));
    printf("%d mandando time_records\n", pid);
    // --- Send event records ---
    int eventCount = event_records.size();
    // Tag '2' identifies event record messages.
    tag = 2;
    if (eventCount > 0)
      bsp_send(
          0, &tag, event_records.data(), eventCount * sizeof(SSEventRecord)
      );

    printf("%d mandando event_records\n", pid);
  }
  bsp_sync();

  // Processor 0 receives the messages from all other processors.
  if (pid == 0)
  {

    printf("%d empezando a recibir\n", pid);
    bsp_nprocs_t numMsgs;
    bsp_size_t totalPayload;
    bsp_qsize(&numMsgs, &totalPayload);
    // Loop over all received messages.
    printf("numMsgs %d parseando mensajes\n", numMsgs);
    for (bsp_nprocs_t i = 0; i < numMsgs; i++)
    {
      bsp_size_t payloadSize;
      int receivedTag;
      // Retrieve the tag and payload size from the next message.
      bsp_get_tag(&payloadSize, &receivedTag);
      printf("payloadSize %d tag %d\n", payloadSize, receivedTag);
      // Only process nonempty messages.
      if (payloadSize > 0)
      {
        if (receivedTag == 1)
        {
          // Message contains time records.
          int numRecords = payloadSize / sizeof(SSTimeRecord);
          // Allocate a temporary buffer to receive the records.
          SSTimeRecord *tmp = (SSTimeRecord *)std::malloc(payloadSize);
          if (!tmp)
            bsp_abort("Memory allocation failed for time records\n");
          bsp_move(tmp, payloadSize);
          // Merge received records into our own vector.
          for (int j = 0; j < numRecords; j++)
            this->time_records.push_back(tmp[j]);
          std::free(tmp);
        }
        else if (receivedTag == 2)
        {
          // Message contains event records.
          int numRecords = payloadSize / sizeof(SSEventRecord);
          // SSEventRecord *tmp = (SSEventRecord *)std::malloc(payloadSize);
          std::vector<SSEventRecord> tmp(numRecords);
          std::vector<char> buffer(payloadSize);

          if (buffer.size() != payloadSize)
            bsp_abort("Buffer allocation failed for time records");

          // Move the payload into the buffer.
          bsp_move(buffer.data(), payloadSize);
          // if (!tmp)
          // bsp_abort("Memory allocation failed for event records\n");
          // bsp_move(tmp.data(), payloadSize);
          printf(
              "recibiendo SSEventRecords num_records %d tamaño de "
              "SSEventRecord %d payloadSize %d\n",
              numRecords, (int)sizeof(SSEventRecord), payloadSize
          );

          // Reinterpret the buffer as an array of SSTimeRecord.
          SSEventRecord *recs =
              reinterpret_cast<SSEventRecord *>(buffer.data());
          for (int j = 0; j < numRecords; j++)
          {

            printf("j: %d num_records %d\n", j, numRecords);
            this->event_records.push_back(recs[j]);
          }
        }
        // If other tag values are used, they can be handled here.
      }
    }
  }
  bsp_sync();
}
