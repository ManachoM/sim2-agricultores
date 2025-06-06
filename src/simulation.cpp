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

#include <cstdio>

Simulation::Simulation(
    const double _max_sim_time, const std::string &_config_path
)
    : max_sim_time(_max_sim_time), conf_path(_config_path) {
  this->env = new Environment(this->fel, this->monitor);
  this->initialize_event_handlers();
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
  this->mercado = mercado;
  this->initialize_agents(mercado);

  // Pasamos al ambiente las nuevas estructuras de datos
  this->env->set_consumidores(this->consumidores);
  this->env->set_feriantes(this->feriantes);
  this->env->set_agricultores(this->agricultores);
  this->mercado->reset_index();

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
    /*
    caller = current_event->get_caller_ptr();
    if (caller != nullptr) {
      caller->process_event(current_event);
      // delete current_event;
      this->fel->event_pool.release(current_event);
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
    */
    this->event_handlers.handle(this, current_event);
    this->fel->event_pool.release(current_event);
    // delete current_event;
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
  // Si no es agricultor de riesgo, nos da lo mismo las amenazas
  if (agricultor_type != "risk")
    return;

  // Inicializamos la info para amenazas

  std::ifstream frost(config["frost_file"].get<std::string>());
  this->env->set_heladas_nivel(json::parse(frost));

  std::ifstream oc(config["oc_file"].get<std::string>());
  json oc_json = json::parse(oc);
  this->env->set_oc_nivel(oc_json["levels"].get<std::vector<int>>());

  std::ifstream spi(config["spi_file"].get<std::string>());
  json spi_json = json::parse(spi);
  this->env->set_sequias_nivel(spi_json["levels"].get<std::vector<int>>());

  printf(
      "Consu array: %ld - Ferr array: %ld - Agr Array: %ld\n",
      consumidores_arr.size(), feriante_arr.size(), agricultor_arr.size()
  );
}

void Simulation::initialize_event_handlers() {
  printf("Inicializando event handlers...\n");
  // AMBIENTE Events
  event_handlers.register_handler(
      AGENT_TYPE::AMBIENTE, EVENTOS_AMBIENTE::INICIO_FERIA,
      [](Simulation *sim, Event *e) {
        // Environment events are always processed locally
        sim->env->process_event(e);
      }
  );

  event_handlers.register_handler(
      AGENT_TYPE::AMBIENTE, EVENTOS_AMBIENTE::FIN_FERIA,
      [](Simulation *sim, Event *e) { sim->env->process_event(e); }
  );

  event_handlers.register_handler(
      AGENT_TYPE::AMBIENTE, EVENTOS_AMBIENTE::CALCULO_PRECIOS,
      [](Simulation *sim, Event *e) { sim->env->process_event(e); }
  );

  event_handlers.register_handler(
      AGENT_TYPE::AMBIENTE, EVENTOS_AMBIENTE::LIMPIEZA_MERCADO_MAYORISTA,
      [](Simulation *sim, Event *e) { sim->env->process_event(e); }
  );

  // AGRICULTOR Events
  event_handlers.register_handler(
      AGENT_TYPE::AGRICULTOR, EVENTOS_AGRICULTOR::CULTIVO_TERRENO,
      [](Simulation *sim, Event *e) {
        // Standard agricultor event - process locally
        Agricultor *agr = sim->agricultor_arr.at(e->get_caller_id());
        agr->process_event(e);
      }
  );

  event_handlers.register_handler(
      AGENT_TYPE::AGRICULTOR, EVENTOS_AGRICULTOR::COSECHA,
      [](Simulation *sim, Event *e) {
        Agricultor *agr = sim->agricultor_arr[e->get_caller_id()];
        agr->process_event(e);
      }
  );

  event_handlers.register_handler(
      AGENT_TYPE::AGRICULTOR, EVENTOS_AGRICULTOR::VENTA_FERIANTE,
      [](Simulation *sim, Event *e) {
        Message msg = e->get_message();
        int prod_id = (int)msg.msg.at("prod_id");
        int amount = (int)msg.msg.at("amount");

        // Get available agricultores for this product
        std::vector<int> agros_id =
            sim->mercado->get_agricultor_por_prod(prod_id);

        // Find an agricultor with sufficient inventory
        Agricultor *agro;
        bool found_valid_inventory = false;

        for (const auto &agr_id : agros_id) {
          agro = sim->agricultor_arr[agr_id];
          Inventario inv = agro->get_inventory_by_id(prod_id);
          double quantity = inv.get_quantity();

          // Skip if inventory is invalid or insufficient
          if (!inv.is_valid_inventory() || quantity < amount)
            continue;

          found_valid_inventory = true;
          agro->process_event(e);
          return;
        }
      }
  );

  event_handlers.register_handler(
      AGENT_TYPE::AGRICULTOR, EVENTOS_AGRICULTOR::INVENTARIO_VENCIDO,
      [](Simulation *sim, Event *e) {
        // Standard event - process locally
        Agricultor *agr = sim->agricultor_arr.at(e->get_caller_id());
        agr->process_event(e);
      }
  );

  // FERIANTE Events
  event_handlers.register_handler(
      AGENT_TYPE::FERIANTE, EVENTOS_FERIANTE::COMPRA_MAYORISTA,
      [](Simulation *sim, Event *e) {
        Message msg = e->get_message();
        // Standard feriante event - process locally
        int feriante_id = e->get_caller_id();
        sim->feriante_arr.at(feriante_id)->process_event(e);
      }
  );

  event_handlers.register_handler(
      AGENT_TYPE::FERIANTE, EVENTOS_FERIANTE::VENTA_CONSUMIDOR,
      [](Simulation *sim, Event *e) {
        // Standard feriante event - process locally
        int feriante_id = e->get_caller_id();
        sim->feriante_arr.at(feriante_id)->process_event(e);
      }
  );

  event_handlers.register_handler(
      AGENT_TYPE::FERIANTE, EVENTOS_FERIANTE::PROCESS_COMPRA_MAYORISTA,
      [](Simulation *sim, Event *e) {
        int agent_id = e->get_caller_id();
        Feriante *fer = sim->feriante_arr.at(agent_id);
        fer->process_event(e);
      }
  );

  // CONSUMIDOR Events
  event_handlers.register_handler(
      AGENT_TYPE::CONSUMIDOR, EVENTOS_CONSUMIDOR::BUSCAR_FERIANTE,
      [](Simulation *sim, Event *e) {
        // Consumidor events are always local
        Agent *caller = e->get_caller_ptr();
        caller->process_event(e);
      }
  );

  event_handlers.register_handler(
      AGENT_TYPE::CONSUMIDOR, EVENTOS_CONSUMIDOR::INIT_COMPRA_FERIANTE,
      [](Simulation *sim, Event *e) {
        Agent *caller = e->get_caller_ptr();
        caller->process_event(e);
      }
  );

  event_handlers.register_handler(
      AGENT_TYPE::CONSUMIDOR, EVENTOS_CONSUMIDOR::PROCESAR_COMPRA_FERIANTE,
      [](Simulation *sim, Event *e) {
        Agent *caller = e->get_caller_ptr();
        caller->process_event(e);
      }
  );

  event_handlers.register_handler(
      AGENT_TYPE::CONSUMIDOR, EVENTOS_CONSUMIDOR::COMPRA_FERIANTE,
      [](Simulation *sim, Event *e) {
        Agent *caller = e->get_caller_ptr();
        caller->process_event(e);
      }
  );

  // Default handler for any unregistered event types
  event_handlers.set_default_handler([](Simulation *sim, Event *e) {
    int agent_type = e->get_type();
    int caller_id = e->get_caller_id();

    // Fall back to the standard event processing based on agent type
    switch (agent_type) {
    case AGENT_TYPE::AMBIENTE:
      sim->env->process_event(e);
      break;

    case AGENT_TYPE::AGRICULTOR: {
      if (caller_id >= 0 && caller_id < sim->agricultor_arr.size()) {
        Agricultor *agr = sim->agricultor_arr.at(caller_id);
        agr->process_event(e);
      } else {
        printf("Warning: Invalid agricultor ID %d\n", caller_id);
      }
      break;
    }

    case AGENT_TYPE::FERIANTE: {
      if (caller_id >= 0 && caller_id < sim->feriante_arr.size()) {
        Feriante *fer = sim->feriante_arr.at(caller_id);
        fer->process_event(e);
      } else {
        printf("Warning: Invalid feriante ID %d\n", caller_id);
      }
      break;
    }

    case AGENT_TYPE::CONSUMIDOR: {
      // Try to use caller_ptr first for safety
      Agent *caller = e->get_caller_ptr();
      if (caller) {
        caller->process_event(e);
      } else if (caller_id >= 0 && caller_id < sim->consumidores_arr.size()) {
        Consumidor *con = sim->consumidores_arr.at(caller_id);
        con->process_event(e);
      } else {
        printf("Warning: Invalid consumidor ID %d\n", caller_id);
      }
      break;
    }

    default:
      printf("Warning: Unknown agent type %d\n", agent_type);
      break;
    }
  });
  printf("Event handlers inicializados!\n");
}

Simulation::~Simulation() {
  printf("Borrando maps...\n");
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

  printf("Borrando arreglos...\n");
  this->feria_arr.resize(0);
  this->consumidores_arr.resize(0);
  this->feriante_arr.resize(0);
  this->agricultor_arr.resize(0);
  // delete this->monitor;
  // delete this->env;

  printf("Borrando FEL...\n");
  delete this->fel;
}
