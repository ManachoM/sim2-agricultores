#include "../includes/environment.h"
#include "../includes/consumidor.h"
#include "../includes/agricultor_factory.h"
#include "../includes/feriante_factory.h"
#include "../includes/postgres_aggregated_monitor.h"

Environment::Environment(FEL *_fel, Monitor *_monitor) : fel(_fel), monitor(_monitor)
{
  // Traemos la configuración de la instancia
  json conf = SimConfig::get_instance("")->get_config();
  // std::string connection_string = conf["DB_URL"].get<std::string>();

  // std::string out_path = conf["out_path"].get<std::string>();
  // this->monitor = new AggregationMonitor(out_path, true);
  // // this->monitor = new PostgresAggregatedMonitor(connection_string);
  // std::cout << "init_size:  " << this->productos.size() << "\n";
  // std::cout << "THIS ENV ptr: " << this << "\n";

  this->initialize_system();
}

void Environment::set_feriantes(std::unordered_map<int, Feriante *> _feriantes)
{
  this->feriantes = _feriantes;
}

void Environment::set_consumidores(std::unordered_map<int, Consumidor *> _cons)
{
  this->consumidores = _cons;
}

void Environment::set_ferias(std::unordered_map<int, Feria *> _ferias)
{
  this->ferias = _ferias;
}

void Environment::process_event(Event *e)
{
  // printf("Procesando evento del ambiente...");
  switch (e->get_process())
  {
  case EVENTOS_AMBIENTE::INICIO_FERIA:
  { /* code */
    this->fel->insert_event(7 * 24.0, AGENT_TYPE::AMBIENTE, EVENTOS_AMBIENTE::INICIO_FERIA, 0, Message(), nullptr);

    // Inicializamos las ferias
    for (const auto &it : this->ferias)
    {
      if (it.second->is_active())
        it.second->initialize_feria();
    }

    // Inicializamos los consumidores
    auto consumidores_por_dia = this->consumidor_dia.find(this->get_day_week() % 7);
    if (consumidores_por_dia == this->consumidor_dia.end())
    {
      fprintf(stderr, "[ERROR] - No hay consumidores que asistan a ferias ese día %d.\n", this->get_day_week() % 7);
      break;
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::exponential_distribution<> d(2);
    for (auto consumidor : consumidores_por_dia->second)
    {
      double purchase_time = d(gen);

      this->fel->insert_event(
          purchase_time, AGENT_TYPE::CONSUMIDOR, EVENTOS_CONSUMIDOR::COMPRA_FERIANTE, consumidor->get_id(), Message(), consumidor);
    }
    break;
  }
  default:
    break;
  }
}

short Environment::get_day_week()
{
  return (short)(this->fel->get_time() / 24) % 7;
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

std::unordered_map<int, Feria *> Environment::get_ferias()
{
  return this->ferias;
}

std::unordered_map<int, Feriante *> Environment::get_feriantes() const { return this->feriantes; }

std::unordered_map<int, Agricultor *> Environment::get_agricultores() { return this->agricultores; }

std::unordered_map<int, Agricultor *> Environment::get_agricultores_rel() { return this->agricultor_por_id_relativo; }

std::unordered_map<int, std::vector<Producto *>> Environment::get_venta_producto_mes() { return this->venta_producto_mes; }

std::unordered_map<int, std::vector<Producto *>> Environment::get_siembra_producto_mes() { return this->siembra_producto_mes; }

int Environment::get_nivel_heladas() { return -1; }

int Environment::get_nivel_sequias() { return this->sequias_nivel.at(this->get_month()); }

int Environment::get_nivel_olas_calor() { return this->oc_nivel.at(this->get_month()); }

void Environment::read_products()
{
  json config = SimConfig::get_instance("")->get_config();

  /** Para productos */
  std::ifstream prods_f(config["prod_file"].get<std::string>());
  json prods = json::parse(prods_f);
  printf("Inicializando productos\n");
  std::unordered_map<int, Producto *> prod_map;
  for (auto it : prods)
  {

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
    prod_map[p->get_id()] = p;
  }
  this->productos = prod_map;
  printf("Creando índice invertido\n");

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
}

void Environment::read_ferias()
{
  json config = SimConfig::get_instance("")->get_config();

  /** Para consumidores y feriantes*/
  std::ifstream ferias_f(config["ferias_file"].get<std::string>());
  json ferias_json = json::parse(ferias_f);
  int cant_ferias = 0;
  int cant_puestos = 0;

  printf("Creando ferias y consumidores\n");

  for (auto feria : ferias_json)
  {
    ++cant_ferias;
    std::vector<int> dias_funcionamiento = feria["dias"].get<std::vector<int>>();

    std::map<int, Feriante *> current_feriantes;
    cant_puestos += feria["cantidad_puestos"].get<int>();
    auto fer = new Feria(dias_funcionamiento, feria["cantidad_puestos"].get<int>(), this, this->fel);
    this->ferias.insert({fer->get_id(), fer});
    this->feria_arr.push_back(fer);
  }
}

void Environment::read_terrenos()
{
  printf("Inicializando sistema...\n");

  json config = SimConfig::get_instance("")->get_config();

  /** Vamos por los terrenos*/
  std::ifstream terr_f(config["terrenos_file"].get<std::string>());
  json terrenos_json = json::parse(terr_f);
  printf("Inicializando agricultores\n");

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

    this->terrenos.insert({terr->get_id(), terr});
  }
}

void Environment::initialize_agents(MercadoMayorista *_mer)
{

  json config = SimConfig::get_instance("")->get_config();

  printf("Inicializando agentes...\t");
  printf("Feriantes -\t");
  // Inicializamos feriantes y consumidores
  for (auto feria : this->ferias)
  {
    // std::cout << "cantidad de terrenos: " << this->terrenos.size() << "\n";
    std::map<int, Feriante *> current_feriantes;
    auto feriante_factory = FerianteFactory(this->fel, this, this->monitor, _mer);
    std::string feriante_type = config["tipo_feriante"].get<std::string>();
    Feriante *feriante;
    for (int i = 0; i < feria.second->get_num_feriantes(); ++i)
    {

      feriante = feriante_factory.create_feriante(feriante_type, feria.first);
      current_feriantes.insert({feriante->get_id(), feriante});
      this->feriantes.insert({feriante->get_id(), feriante});
      this->feriante_arr.push_back(feriante);
    }

    feria.second->set_feriantes(current_feriantes);

    int consumidores_por_puesto = config["consumidor_feriante_ratio"].get<int>();
    std::string consumer_type = config["tipo_consumidor"].get<std::string>();
    int cantidad_consumidores = consumidores_por_puesto * feria.second->get_num_feriantes();

    auto cons_factory = ConsumidorFactory(this->fel, this, this->monitor);

    for (int i = 0; i < cantidad_consumidores; ++i)
    {
      auto cons = cons_factory.create_consumidor(consumer_type, feria.first);
      this->consumidores.insert({cons->get_id(), cons});
      this->consumidores_arr.push_back(cons);
    }
  }

  // Creamos el índice de consumidores por día
  for (auto const &el : this->consumidores)
  {
    Feria *feria;
    try
    {
      feria = this->ferias.at(el.second->get_feria());
    }
    catch (const std::out_of_range &e)
    {
      fprintf(stderr, "[ERROR] - El consumidor con id %d asiste a una feria que no existe (id %d)\n", el.second->get_id(), el.second->get_feria());
      continue;
    }

    double dia_feria = feria->get_next_active_time();

    auto busqueda_entrada = this->consumidor_dia.find((int)dia_feria);

    // Si no existe, creamos la entrada
    if (busqueda_entrada == this->consumidor_dia.end())
    {
      std::vector<Consumidor *> arr;
      arr.push_back(el.second);
      this->consumidor_dia.insert({dia_feria, arr});
      continue;
    }

    // Si existe, simplemente anexamos
    busqueda_entrada->second.push_back(el.second);
  }

  std::string agricultor_type = config["tipo_agricultor"].get<std::string>();
  Agricultor *agr;
  auto agricultor_factory = AgricultorFactory(this->fel, this, this->monitor, _mer);
  printf("Agricultores \n");

  for (auto terr : this->terrenos)
  {
    agr = agricultor_factory.create_agricultor(agricultor_type, terr.second);
    this->agricultores.insert({agr->get_id(), agr});
    this->agricultor_arr.push_back(agr);
  }

  std::cout << "Cantidad de agricultores " << this->agricultores.size() << std::endl;
}

void Environment::initialize_system()
{

  // Leemos desde archivos y generamos objetos estáticos
  this->read_products();

  this->read_ferias();

  this->read_terrenos();

  // Creamos mercado mayorista
  auto *mercado = new MercadoMayorista(this);

  // TODO: Cargar amenazas

  // Generamos los agentes para la simulación
  this->initialize_agents(mercado);

  mercado->reset_index();

  // Inicializamos los eventos del ambiente
  for (int i = 0; i < 7; ++i)
  {
    this->fel->insert_event(24.0 * i, AGENT_TYPE::AMBIENTE, EVENTOS_AMBIENTE::INICIO_FERIA, 0, Message());
  }
}

Environment::~Environment() = default;

std::unordered_map<int, Producto *> Environment::get_productos()
{
  return this->productos;
}

std::unordered_map<int, Consumidor *> Environment::get_consumidores()
{
  return this->consumidores;
}

Consumidor *Environment::get_consumidor(int consumidor_id)
{
  return this->consumidores_arr[consumidor_id];
}

Feriante *Environment::get_feriante(int feriante_id)
{
  return this->feriante_arr[feriante_id];
}

Agricultor *Environment::get_agricultor(int agro_id)
{
  return this->agricultor_arr[agro_id];
}

Feria *Environment::get_feria(int feria_id)
{
  return this->feria_arr[feria_id];
}