#include "../includes/simulation.h"

#include "../includes/agricultor.h"
#include "../includes/agricultor_factory.h"
#include "../includes/environment.h"
#include "../includes/feria.h"
#include "../includes/feriante_factory.h"
#include "../includes/heap_fel.h"
#include "../includes/mercado_mayorista.h"
#include "../includes/monitor.h"
#include "../includes/postgres_aggregated_monitor.h"
#include "../includes/sim_config.h"

Simulation::Simulation(
    const double _max_sim_time, const std::string &_config_path
)
    : max_sim_time(_max_sim_time), conf_path(_config_path) {
  this->env = new Environment(this->fel, this->monitor);
}

void Simulation::run() {
  // Leemos los archivos de entrada e
  // instanciamos agentes
  this->read_products();
  this->env->set_productos(this->productos);
  this->read_ferias();
  this->env->set_ferias(this->ferias);
  this->read_terrenos();

  auto mercado = new MercadoMayorista(this->env);

  this->initialize_agents(mercado);

  // Pasamos al ambiente las nuevas estructuras de datos
  this->env->set_consumidores(this->consumidores);
  this->env->set_feriantes(this->feriantes);
  this->env->set_agricultores(this->agricultores);
  mercado->reset_index();

  // Inicializamos los eventos del ambiente
  for (int i = 1; i < 7; ++i) {
    this->fel->insert_event(
        24.0 * i, AGENT_TYPE::AMBIENTE, EVENTOS_AMBIENTE::INICIO_FERIA, 0,
        Message()
    );
  }

  Event *current_event;
  Agent *caller;

  std::map<std::string, int> agent_type_count = {
      {"CONSUMIDOR", 0}, {"FERIANTE", 0}, {"AGRICULTOR", 0}, {"AMBIENTE", 0}
  };
  std::string agent_type;
  auto start_time = std::chrono::high_resolution_clock::now();

  while (fel->get_time() <= this->max_sim_time && !fel->is_empty()) {
    current_event = fel->next_event();
    agent_type = agent_type_to_agent.at(current_event->get_type());
    ++agent_type_count[agent_type];

    if ((current_event->event_id % 1'000'000) == 0)
      std::cout << "SIM TIME: " << current_event->get_time()
                << "\n EVENT ID: " << current_event->event_id << "\n";

    caller = current_event->get_caller_ptr();
    if (caller != nullptr) {
      caller->process_event(current_event);
      delete current_event;
      continue;
    }
    switch (current_event->get_type()) {
    case AGENT_TYPE::AMBIENTE: {
      this->env->process_event(current_event);
      continue;
    }
    case AGENT_TYPE::AGRICULTOR: {
      caller = this->agricultor_arr[current_event->get_caller_id()];
      break;
    }
    case AGENT_TYPE::FERIANTE: {
      caller = this->feriante_arr[current_event->get_caller_id()];
      break;
    }
    case AGENT_TYPE::CONSUMIDOR: {
      caller = this->consumidores_arr[current_event->get_caller_id()];
      break;
    }
    default:
      break;
    }

    caller->process_event(current_event);
    delete current_event;
  }

  auto end_time = std::chrono::high_resolution_clock::now();

  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
      end_time - start_time
  );
  monitor->write_duration((double)duration.count());
  monitor->write_results();
  // Memory deletion
  std::cout << "SIM DURATION: " << duration.count() << "[ms]\n";
  delete env;

  for (auto const &[tipo, cantidad] : agent_type_count) {
    std::cout << "AGENTE " << tipo
              << " \t CANTIDAD DE EVENTOS PROCESADOS: " << cantidad << "\n";
    monitor->write_params(tipo, std::to_string(cantidad));
  }
  delete monitor;

  printf("ÚLTIMO EVENTO EN COLA %d\n", fel->next_event()->event_id);
  printf("[FIN SIMLUACION]\n");
}

void Simulation::read_products() {
  json config = SimConfig::get_instance()->get_config();
  /** Para productos */
  std::ifstream prods_f(config["prod_file"].get<std::string>());
  json prods = json::parse(prods_f);
  printf("Inicializando productos\n");
  std::unordered_map<int, Producto *> prod_map;
  for (auto it : prods) {
    std::vector<int> meses_siembra =
        it["meses_siembra"].get<std::vector<int>>();
    std::vector<int> meses_venta = it["meses_venta"].get<std::vector<int>>();

    UNIT_TYPES unit;
    if (it["unidad"] == "unidades") {
      unit = UNIT_TYPES::UNIDAD;
    } else {
      unit = UNIT_TYPES::KILOS;
    }

    Producto *p = new Producto(
        it["nombre"].get<std::string>(), meses_siembra, meses_venta,
        it["dias_cosecha"].get<int>(), unit, it["unit/ha"].get<double>(),
        it["volumen_feriante"].get<double>(),
        it["volumen_un_consumidor"].get<double>(),
        it["prob_consumir"].get<double>(), it["precio_consumidor"].get<double>()
    );

    std::vector<int> hel = it["heladas"].get<std::vector<int>>();
    std::vector<int> seq = it["sequias"].get<std::vector<int>>();
    std::vector<int> oc = it["oc"].get<std::vector<int>>();
    std::vector<int> pl = it["plagas"].get<std::vector<int>>();
    std::vector<double> pms = it["precios_mes"].get<std::vector<double>>();
    double costo_ha = it["costo_ha"].get<double>();
    p->set_heladas(hel);
    p->set_sequias(seq);
    p->set_olas_calor(oc);
    p->set_plagas(pl);
    p->set_precios_mes(pms);
    p->set_costo_ha(costo_ha);
    prod_map[p->get_id()] = p;
  }

  this->productos = prod_map;
}

void Simulation::read_ferias() {
  json config = SimConfig::get_instance()->get_config();

  std::unordered_map<int, Feria *> ferias;
  /** Para consumidores y feriantes*/
  std::ifstream ferias_f(config["ferias_file"].get<std::string>());
  json ferias_json = json::parse(ferias_f);
  int cant_ferias = 0;
  int cant_puestos = 0;

  printf("Creando ferias y consumidores\n");

  for (auto feria : ferias_json) {
    ++cant_ferias;
    std::vector<int> dias_funcionamiento =
        feria["dias"].get<std::vector<int>>();

    std::map<int, Feriante *> current_feriantes;
    cant_puestos += feria["cantidad_puestos"].get<int>();
    auto fer = new Feria(
        dias_funcionamiento, feria["cantidad_puestos"].get<int>(), this->env,
        this->fel
    );
    ferias.insert({fer->get_id(), fer});
    this->feria_arr.push_back(fer);
  }

  this->ferias = ferias;
}

void Simulation::read_terrenos() {

  json config = SimConfig::get_instance()->get_config();

  /** Vamos por los terrenos*/
  std::ifstream terr_f(config["terrenos_file"].get<std::string>());
  json terrenos_json = json::parse(terr_f);
  printf("Inicializando terrenos\n");

  for (auto terreno : terrenos_json) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> d(0, this->productos.size() - 1);
    int prod;
    prod = d(gen);

    auto terr = new Terreno(
        terreno["cod_comuna"].get<int>(), terreno["area_ha"].get<double>(),
        terreno["comuna"].get<std::string>(), prod
    );

    this->terrenos.insert({terr->get_id(), terr});
  }
}

void Simulation::initialize_agents(MercadoMayorista *_mer) {
  json config = SimConfig::get_instance()->get_config();

  printf("[INICIALIZANDO AGENTES]t");
  printf("Feriantes -\t");
  // Inicializamos feriantes y consumidores
  for (auto feria : this->ferias) {
    // std::cout << "cantidad de terrenos: " << this->terrenos.size() << "\n";
    std::map<int, Feriante *> current_feriantes;
    auto feriante_factory =
        FerianteFactory(this->fel, this->env, this->monitor, _mer);
    std::string feriante_type = config["tipo_feriante"].get<std::string>();
    Feriante *feriante;
    for (int i = 0; i < feria.second->get_num_feriantes(); ++i) {

      feriante = feriante_factory.create_feriante(feriante_type, feria.first);
      current_feriantes.insert({feriante->get_id(), feriante});
      this->feriantes.insert({feriante->get_id(), feriante});
      this->feriante_arr.push_back(feriante);
    }

    feria.second->set_feriantes(current_feriantes);

    int consumidores_por_puesto =
        config["consumidor_feriante_ratio"].get<int>();
    std::string consumer_type = config["tipo_consumidor"].get<std::string>();
    int cantidad_consumidores =
        consumidores_por_puesto * feria.second->get_num_feriantes();

    auto cons_factory = ConsumidorFactory(this->fel, this->env, this->monitor);

    for (int i = 0; i < cantidad_consumidores; ++i) {
      auto cons = cons_factory.create_consumidor(consumer_type, feria.first);
      this->consumidores.insert({cons->get_id(), cons});
      this->consumidores_arr.push_back(cons);
    }
  }

  std::string agricultor_type = config["tipo_agricultor"].get<std::string>();
  Agricultor *agr;
  auto agricultor_factory =
      AgricultorFactory(this->fel, this->env, this->monitor, _mer);
  printf("Agricultores \n");

  for (auto terr : this->terrenos) {
    agr = agricultor_factory.create_agricultor(agricultor_type, terr.second);
    this->agricultores.insert({agr->get_id(), agr});
    this->agricultor_arr.push_back(agr);
  }

  std::cout << "Cantidad de agricultores " << this->agricultores.size()
            << std::endl;
}

Simulation::~Simulation() {
  for (const auto &pair : this->consumidores)
    delete pair.second;
  for (const auto &pair : this->ferias)
    delete pair.second;
  for (const auto &pair : this->agricultores)
    delete pair.second;
  for (const auto &pair : this->feriantes)
    delete pair.second;
  for (const auto &pair : this->productos)
    delete pair.second;

  this->feria_arr.resize(0);
  this->consumidores_arr.resize(0);
  this->feriante_arr.resize(0);
  this->agricultor_arr.resize(0);
  // delete this->monitor;
  // delete this->env;
  delete this->fel;
}
