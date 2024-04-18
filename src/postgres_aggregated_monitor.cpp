#include "../includes/postgres_aggregated_monitor.h"

#include "../includes/sim_config.h"

#include <ctime>

std::string const insert_execution = ("INSERT INTO execution(execution_status_id, start_time) "
                                      "VALUES (1, to_timestamp($1, 'YYYY-MM-DDT0HH24:MI:SSZ')) "
                                      "RETURNING execution_id ");

std::string const insert_execution_params = "INSERT INTO execution_params (execution_id, param_name, param_value) VALUES ($1, $2, $3)";

std::string const update_execution_duration = "UPDATE execution SET execution_time = $1 WHERE execution_id = $2";

std::string const update_finished_execution = "UPDATE execution SET execution_status_id = 2 WHERE execution_id = $1";

std::string const update_failed_execution = "UPDATE execution SET execution_status_id = 3 WHERE execution_status_id != 2 AND execution_id = $1";

std::string const insert_aggregated_product_result = "INSERT INTO aggregated_product_results (execution_id, process, time, product_id, value) VALUES ($1, $2, $3, $4, $5)";

PostgresAggregatedMonitor::PostgresAggregatedMonitor(std::string const &db_url, bool _debug) : Monitor("", _debug), _database_url(db_url)
{
    json conf = SimConfig::get_instance("")->get_config();

    if (this->_database_url == "")
        this->_database_url = conf["DB_URL"].get<std::string>();

    // Generamos la conexión
    pqxx::connection conn(this->_database_url);

    // Preparamos las queries
    conn.prepare("insert_execution", insert_execution);
    conn.prepare("insert_exec_param", insert_execution_params);

    // Registramos la ejecución y sus parámetros
    pqxx::work t{conn};
    std::string now = get_current_timestamp().c_str();

    this->execution_id = t.exec_prepared1("insert_execution", get_current_timestamp().c_str())[0].as<int>();
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
            throw std::logic_error("Tipo de variable de configuración no soportado.\n");
        }

        t.exec_prepared0("insert_exec_param", this->execution_id, it.key().c_str(), str_val.c_str());
    }

    // Commit y cerrar la conexión
    t.commit();

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
}

void PostgresAggregatedMonitor::write_duration(double time)
{
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
    if (log["agent_type"].get<std::string>() == "AGRICULTOR" && log["agent_process"].get<std::string>() == "COSECHA")
    {
        //    printf("LOGGEANDO AGRICULTOR\n");
        double cantidad = log["cantidad_cosechada"].get<double>();
        // printf("aki voi\n");
        // printf("[LOG] - [AGRICULTOR] - [COSECHA] - ID Prod: %d\tCantidad cosechada: %f\t Cantidad acumulada del mes: %f\n ", log["producto_cosechado"].get<int>(), cantidad, this->agg_logs.find("AGRICULTOR")->second.find(month_n)->second.find(std::to_string(log["producto_cosechado"].get<int>()))->second);
        this->agg_logs["AGRICULTOR"][month_n][std::to_string(log["producto_cosechado"].get<int>())] += cantidad;
        // this->aggregated_logs["AGRICULTOR"][year][month][std::to_string(log["producto_cosechado"].get<int>())] = +log["cantidad_cosechada"].get<double>();
    }
    else if (log["agent_type"].get<std::string>() == "FERIANTE" && log["agent_process"].get<std::string>() == "PROCESAR_COMPRA_AGRICULTOR")
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

        std::string target_prod = std::to_string(log["target_product"].get<int>());
        double cantidad = log["target_amount"].get<double>();
        this->agg_logs["FERIANTE"][month_n][target_prod] += cantidad;
    }
    else if (log["agent_type"].get<std::string>() == "CONSUMIDOR" && log["agent_process"].get<std::string>() == "PROCESAR_COMPRA_FERIANTE")
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
        this->agg_logs["CONSUMIDOR"][month_n][target_prod] += log["target_amount"].get<double>();

        // this->aggregated_logs["CONSUMIDOR"][year][month][target_prod] = +it.value()["target_amount"].get<double>();
    }
    else if (log["agent_type"].get<std::string>() == "CONSUMIDOR" && log["agent_process"].get<std::string>() == "COMPRA_FERIATE")
    {
        printf("Procesando nuevo evento del consumidor\n");
        // Obtenemos la lista de compras
        std::vector<json> compras = log["compras"].get<std::vector<json>>();

        for (const json &compra : compras)
        {
            std::string target_prod = std::to_string(compra["target_product"].get<int>());
            this->agg_logs["CONSUMIDOR"][month_n][target_prod] += compra["target_amount"].get<double>();
        }
    }
    else if (log["agent_type"].get<std::string>() == "FERIANTE" && log["agent_process"].get<std::string>() == "COMPRA_MAYORISTA")
    {
        std::vector<json> compras = log["compras"].get<std::vector<json>>();
        for (const json &compra : compras)
        {
            std::string target_prod = std::to_string(compra["target_product"].get<int>());
            double cantidad = compra["target_amount"].get<double>();
            this->agg_logs["FERIANTE"][month_n][target_prod] += cantidad;
        }
    }
    this->last_recorded_month = short(log["time"].get<double>() / 720);
}

void PostgresAggregatedMonitor::write_results()
{
    // Preparamos la conexión y el statement
    pqxx::connection conn(this->_database_url);
    conn.prepare("insert_aggregated_product_result", insert_aggregated_product_result);

    pqxx::work t{conn};
    // Pasaremos por los tres primeros niveles de anidado para poder ser precisos con el mensaje en DB
    for (auto const &[month, cantidad_por_prod] : this->agg_logs["FERIANTE"])
    {
        for (auto const &[prod_id, cantidad] : cantidad_por_prod)
        {
            t.exec_prepared0("insert_aggregated_product_result", this->execution_id, "COMPRA DE FERIANTE A AGRICULTOR", month, prod_id, cantidad);
        }
    }

    for (auto const &[month, cantidad_por_prod] : this->agg_logs["CONSUMIDOR"])
    {
        for (auto const &[prod_id, cantidad] : cantidad_por_prod)
        {
            t.exec_prepared0("insert_aggregated_product_result", this->execution_id, "COMPRA DE CONSUMIDOR", month, prod_id, cantidad);
        }
    }

    for (auto const &[month, cantidad_por_prod] : this->agg_logs["AGRICULTOR"])
    {
        for (auto const &[prod_id, cantidad] : cantidad_por_prod)
        {
            t.exec_prepared0("insert_aggregated_product_result", this->execution_id, "COSECHA AGRICULTOR", month, prod_id, cantidad);
        }
    }
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
