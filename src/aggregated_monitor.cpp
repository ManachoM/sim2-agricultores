#include "../includes/aggregated_monitor.h"

AggregationMonitor::AggregationMonitor(std::string const &_file_prefix, bool _debug) : Monitor(_file_prefix, _debug)
{
    this->agg_logs["AGRICULTOR"] = std::map<int, std::map<std::string, double>>();
    this->agg_logs["FERIANTE"] = std::map<int, std::map<std::string, double>>();
    this->agg_logs["CONSUMIDOR"] = std::map<int, std::map<std::string, double>>();
    // this->aggregated_logs[0][0] = 0;
    for (int i = 0; i < 21; ++i)
    {
        this->agg_logs["AGRICULTOR"][0][std::to_string(i)] = 0;
        this->agg_logs["FERIANTE"][0][std::to_string(i)] = 0;
        this->agg_logs["CONSUMIDOR"][0][std::to_string(i)] = 0;
    }
};

void AggregationMonitor::write_log(json log)
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
            this->aggregated_logs["AGRICULTOR"][year][month][std::to_string(i)] = 0;
            this->aggregated_logs["FERIANTE"][year][month][std::to_string(i)] = 0;
            this->aggregated_logs["CONSUMIDOR"][year][month][std::to_string(i)] = 0;
        }

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
        double cantidad = log["cantidad_cosechada"].get<double>();
        // printf("aki voi\n");
        // printf("[LOG] - [AGRICULTOR] - [COSECHA] - ID Prod: %d\tCantidad cosechada: %f\t Cantidad acumulada del mes: %f\n ", log["producto_cosechado"].get<int>(), cantidad, this->agg_logs.find("AGRICULTOR")->second.find(month_n)->second.find(std::to_string(log["producto_cosechado"].get<int>()))->second);
        this->agg_logs["AGRICULTOR"][month_n][std::to_string(log["producto_cosechado"].get<int>())] += cantidad;
        this->aggregated_logs["AGRICULTOR"][year][month][std::to_string(log["producto_cosechado"].get<int>())] = +log["cantidad_cosechada"].get<double>();
    }
    else if (log["agent_type"].get<std::string>() == "FERIANTE" && log["agent_process"].get<std::string>() == "COMPRA MAYORISTA")
    {
        auto compras = log["compras"];
        for (json::iterator it = compras.begin(); it != compras.end(); ++it)
        {

            // Si el id de agricultor es -1, no se concretó una compra
            try
            {
                if (it.value()["id_agricultor"].get<int>() == -1)
                    continue;
            }
            catch (const nlohmann::detail::type_error e)
            {
                std::cerr << it.value().dump(4) << '\n';
                std::cerr << e.what() << '\n';
                continue;
            }

            std::string target_prod = std::to_string(it.value()["target_product"].get<int>());
            double cantidad = it.value()["target_amount"].get<double>();
            this->agg_logs["FERIANTE"][month_n][target_prod] += cantidad;

            this->aggregated_logs["FERIANTE"][year][month][target_prod] = +cantidad;
        }
    }
    else if (log["agent_type"].get<std::string>() == "CONSUMIDOR" && log["agent_process"].get<std::string>() == "COMPRA FERIANTE")
    {

        auto compras = log["compras"];
        for (json::iterator it = compras.begin(); it != compras.end(); ++it)
        {
            // Si el id de feriante es -1, no se concretó una compra
            if (it.value()["feriante_id"].get<int>() == -1)
                continue;
            std::string target_prod = std::to_string(it.value()["target_product"].get<int>());
            this->agg_logs["CONSUMIDOR"][month_n][target_prod] += it.value()["target_amount"].get<double>();

            this->aggregated_logs["CONSUMIDOR"][year][month][target_prod] = +it.value()["target_amount"].get<double>();
        }
    }
    this->last_recorded_month = short(log["time"].get<double>() / 720);
}

AggregationMonitor::~AggregationMonitor()
{
    json out_json;
    {
        json aux;
        for (auto it = this->agg_logs["AGRICULTOR"].begin(); it != this->agg_logs["AGRICULTOR"].end(); ++it)
        {
            json nested;
            for (auto element = it->second.begin(); element != it->second.end(); ++element)
            {
                nested[element->first] = element->second;
            }
            aux[it->first] = nested;
        }
        out_json["AGRICULTOR"] = aux;
    }

    {
        json aux;
        for (auto it = this->agg_logs["FERIANTE"].begin(); it != this->agg_logs["FERIANTE"].end(); ++it)
        {
            json nested;
            for (auto element = it->second.begin(); element != it->second.end(); ++element)
            {
                nested[element->first] = element->second;
            }
            aux[it->first] = nested;
        }
        out_json["FERIANTE"] = aux;
    }

    {
        json aux;
        for (auto it = this->agg_logs["CONSUMIDOR"].begin(); it != this->agg_logs["CONSUMIDOR"].end(); ++it)
        {
            json nested;
            for (auto element = it->second.begin(); element != it->second.end(); ++element)
            {
                nested[element->first] = element->second;
            }
            aux[it->first] = nested;
        }
        out_json["CONSUMIDOR"] = aux;
    }
    std::string out_dir = "out/" + this->file_prefix + "_" + this->sim_id + "_aggregated.json";
    if (system("mkdir -p out/"))
    {
        fprintf(stderr, "[ERROR] - No se pudo crear directorio de salida. Verifique los permisos sobre los directorios del sistema.\nA continuación se dejan los resultados de la simulación:\n");
        printf("%s\n", this->aggregated_logs.dump(4).c_str());
        exit(EXIT_FAILURE);
    }

    std::ofstream out;
    printf("Destruyendo monitor... OUTPUT string %s\n", out_dir.c_str());
    out.open(out_dir);
    out << out_json.dump(2);
    out << "\n";
    out.close();
}