#include "../includes/environment.h"

Environment::Environment(FEL *_fel) : fel(_fel)
{
  // Traemos la configuración de la instancia
  json conf = SimConfig::get_instance("")->get_config();
  std::string out_path = conf["out_path"].get<std::string>();
  this->monitor = new AggregationMonitor(out_path, true);

  this->initialize_system();
}

void Environment::set_feriantes(std::map<int, Feriante *> _feriantes)
{
  this->feriantes = _feriantes;
}

void Environment::set_consumidores(std::map<int, Consumidor *> _cons)
{
  this->consumidores = _cons;
}

void Environment::set_ferias(std::map<int, Feria *> _ferias)
{
  this->ferias = _ferias;
}

void Environment::process_event(Event *e)
{
  printf("Procesando evento del ambiente...");
}

short Environment::get_day_week()
{
  return (short)(this->fel->get_time() / 24) % 24;
}

short Environment::get_day_month()
{
  return (short)(this->fel->get_time() / 24.0) % 30;
}

short Environment::get_month()
{
  return (short)(this->fel->get_time() / 720) % 12;
}

short Environment::get_year()
{
  return ((short)this->fel->get_time() / 8640);
}

std::map<int, Feria *> Environment::get_ferias() { return this->ferias; }

std::map<int, Feriante *> Environment::get_feriantes() { return this->feriantes; }

std::map<int, Agricultor *> Environment::get_agricultores() { return this->agricultores; }

std::map<int, Producto *> Environment::get_productos() { return this->productos; }

std::map<int, std::vector<Producto *>> Environment::get_venta_producto_mes() { return this->venta_producto_mes; }

std::map<int, std::vector<Producto *>> Environment::get_siembra_producto_mes() { return this->siembra_producto_mes; }

int Environment::get_nivel_heladas() { return -1; }

int Environment::get_nivel_sequias() { return this->sequias_nivel.at(this->get_month()); }

int Environment::get_nivel_olas_calor() { return this->oc_nivel.at(this->get_month()); }

void Environment::initialize_system()
{
  printf("Inicializando sistema...\n");

  json config = SimConfig::get_instance("")->get_config();

  // TODO: Generar MM

  /** Para productos */
  std::ifstream prods_f(config["prod_file"].get<std::string>());
  json prods = json::parse(prods_f);

  for (auto it : prods)
  {
    std::cout << it.dump() << '\n';
    std::vector<int> meses_siembra = it["meses_siembra"].get<std::vector<int>>();
    std::vector<int> meses_venta = it["meses_venta"].get<std::vector<int>>();

    UNIT_TYPES unit;
    if (it["unidad"] == "unidades")
    {
      unit = UNIT_TYPES::UNIDAD;
    }
    else
    {
      unit = UNIT_TYPES::KILOS;
    }

    Producto *p = new Producto(
        it["nombre"].get<std::string>(),
        meses_siembra,
        meses_venta,
        it["dias_cosecha"].get<int>(),
        unit,
        it["unit/ha"].get<double>(),
        it["volumen_feriante"].get<double>(),
        it["volumen_un_consumidor"].get<double>(),
        it["prob_consumir"].get<double>());

    std::vector<int> hel = it["heladas"].get<std::vector<int>>();
    std::vector<int> seq = it["sequias"].get<std::vector<int>>();
    std::vector<int> oc = it["oc"].get<std::vector<int>>();
    std::vector<int> pl = it["plagas"].get<std::vector<int>>();
    p->set_heladas(hel);
    p->set_sequias(seq);
    p->set_olas_calor(oc);
    p->set_plagas(pl);
    this->productos.insert({p->get_id(), p});
  }

  // Genreamos el índice invertido de meses de venta
  for (int i = 0; i < 12; ++i)
  {
    std::vector<Producto *> prods;

    // por cada producto
    for (auto p : this->productos)
    {
      // revisamos si el mes actual que estamos revisando está dentro de los meses del producto
      std::vector<int> meses_venta = p.second->get_meses_venta();
      for (int mes : meses_venta)
      {
        if (i == mes)
        {
          prods.push_back(p.second);
          break;
        }
      }
    }
    assert((int)prods.size() > 0);
    this->venta_producto_mes.insert({i, prods});
  }

  // Generamos el índice invertido para los meses de siembra
  for (int i = 0; i < 12; ++i)
  {
    std::vector<Producto *> prods_a_insertar;

    for (auto p : this->productos)
    {
      std::vector<int> meses_siembra = p.second->get_meses_siembra();

      for (int mes : meses_siembra)
      {
        if (mes == i)
        {
          prods_a_insertar.push_back(p.second);
          break;
        }
      }
    }

    assert((int)prods_a_insertar.size() > 0);
    this->siembra_producto_mes.insert({i, prods_a_insertar});
  }
  // TODO: Cargar amenazas
  /** Para consumidores y feriantes*/
  std::ifstream ferias_f(config["ferias_file"].get<std::string>());
  json ferias_json = json::parse(ferias_f);
  int cant_ferias = 0;
  int cant_puestos = 0;

  std::map<int, Consumidor *> consumers; // Para ir almacenando todos los consumidores
  for (auto feria : ferias_json)
  {
    ++cant_ferias;
    std::vector<int> dias_funcionamiento = feria["dias"].get<std::vector<int>>();

    std::map<int, Feriante *> current_feriantes;
    Feriante *feriante;
    cant_puestos += feria["cantidad_puestos"].get<int>();
    auto fer = new Feria(dias_funcionamiento, this, this->fel);
    for (int i = 0; i < feria["cantidad_puestos"].get<int>(); ++i)
    {
      feriante = new Feriante(this->fel, this); // TODO: Arreglar referencia a MM
      feriante->add_feria(fer);
      feriante->set_monitor(this->monitor);
      current_feriantes.insert({feriante->get_id(), feriante});
      this->feriantes.insert({feriante->get_id(), feriante});
    }

    fer->set_feriantes(current_feriantes);

    int consumidores_por_puesto = config["consumidor_feriante_ratio"].get<int>();
    int cant_integrantes = 1;
    // Creamos los consumidores
    for (size_t i = 0; i < consumidores_por_puesto * feria["cantidad_puestos"].get<int>(); ++i)
    {
      auto cons = new Consumidor(this->fel, fer->get_id(), cant_integrantes);
      consumers.insert({cons->get_id(), cons});
    }
  }

  for (int i = 0; i < 7; ++i)
  {
    this->fel->insert_event(24.0 * i, AGENT_TYPE::AMBIENTE, EVENTOS_AMBIENTE::INICIO_FERIA, 0, Message());
  }

  /** Vamos por los terrenos*/
  std::ifstream terr_f(config["terrenos_file"].get<std::string>());
  json terrenos_json = json::parse(terr_f);

  for (auto terreno : terrenos_json)
  {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> d(0, this->productos.size() - 1);
    int prod;
    prod = d(gen);

    auto terr = new Terreno(
        terreno["cod_comuna"].get<int>(),
        terreno["area_ha"].get<double>(),
        terreno["comuna"].get<std::string>(),
        prod);

    // TODOauto agro = new Agricultor()
  }
}

Environment::~Environment() = default;