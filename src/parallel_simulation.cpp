#include "../includes/parallel_simulation.h"

#include "../includes/agricultor.h"
#include "../includes/agricultor_factory.h"
#include "../includes/consumidor_factory.h"
#include "../includes/event.h"
#include "../includes/feriante_factory.h"
#include "../includes/mercado_mayorista.h"
#include "../includes/message_serializer.h"
#include "../includes/product_partitioner.h"
#include "../includes/sim_config.h"
#include "../includes/simmulated_annealing_partitioner.h"

#include <bsp.h>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <time.h>

/**
 * @brief "BalanceTemporal" style objective function for Productos.
 *
 * Higher is better. Penalizes months with zero load in each partition,
 * rewards overall load, tries to keep distribution balanced.
 */
double product_objective_function(
    const std::unordered_map<int, Producto *> &items,
    const std::vector<int> &sol
)
{
  // Example parameters
  const double penalty = 0.1;
  const int time_periods = 12; // months

  // Partition -> monthly load
  std::unordered_map<int, std::vector<double>> partition_load;
  for (size_t i = 0; i < sol.size(); ++i)
  {
    int part_id = sol[i];
    if (partition_load.find(part_id) == partition_load.end())
    {
      partition_load[part_id] = std::vector<double>(time_periods, 0.0);
    }
    const Producto *prod = items.at((int)i);
    for (int month : prod->get_meses_venta())
    {
      if (month >= 0 && month < time_periods)
      {
        partition_load[part_id][month] += prod->get_probabilidad_consumo();
      }
    }
  }

  // Compute total workload & a balance measure
  double total_score = 0.0;
  for (auto &kv : partition_load)
  {
    const auto &monthly = kv.second;
    // Each partition's sum
    double part_sum = 0.0;
    for (double val : monthly)
    {
      if (val > 0.0)
      {
        part_sum += val;
      }
      else
      {
        // penalize idle month
        part_sum -= penalty;
      }
    }
    total_score += part_sum;
  }

  // Optionally, incorporate a cross-partition monthly balance check
  // (like a day-by-day coefficient of variation for each partition).
  // This snippet simply sums up partition scores.

  return total_score;
}

/**
 * @brief "Tiered" style objective function for Ferias focusing on day-balance
 * (Temporal focal distribution) plus total workload.
 */
double feria_objective_function(
    const std::unordered_map<int, Feria *> &items, const std::vector<int> &sol
)
{
  // Weighted components
  const double penalty = 0.1;         // for idle days
  const double weight_balance = 0.7;  // how important day-balance is
  const double weight_workload = 0.3; // how important total workload is
  const int time_periods = 7;         // days in a week

  // Partition -> daily load
  std::unordered_map<int, std::vector<double>> partition_load;
  for (size_t i = 0; i < sol.size(); ++i)
  {
    int part_id = sol[i];
    if (partition_load.find(part_id) == partition_load.end())
    {
      partition_load[part_id] = std::vector<double>(time_periods, 0.0);
    }
    const Feria *feria = items.at((int)i);
    for (int d : feria->get_dia_funcionamiento())
    {
      if (d >= 0 && d < time_periods)
      {
        partition_load[part_id][d] += feria->get_num_feriantes();
      }
    }
  }

  // 1) Coefficient of variation across partitions for each day
  double cv_score = 0.0;
  double total_workload = 0.0;

  // For each day, gather loads from each partition
  for (int day = 0; day < time_periods; ++day)
  {
    std::vector<double> day_loads;
    for (auto &kv : partition_load)
    {
      day_loads.push_back(kv.second[day]);
      total_workload += kv.second[day];
    }
    // Calculate mean
    double sum = std::accumulate(day_loads.begin(), day_loads.end(), 0.0);
    double mean = sum / day_loads.size();

    if (mean > 0.0)
    {
      // std dev
      double sq_diff_sum = 0.0;
      for (double load : day_loads)
      {
        double diff = load - mean;
        sq_diff_sum += diff * diff;
      }
      double stdev = std::sqrt(sq_diff_sum / day_loads.size());
      double cv = stdev / mean;
      // Lower CV is better => negative
      cv_score -= cv;
    }
    else
    {
      // no load at all => penalize
      cv_score -= 1.0;
    }
  }
  // average across the 7 days
  cv_score /= (double)time_periods;

  // 2) Penalty for idle partitions on each day
  double idle_penalty = 0.0;
  for (auto &kv : partition_load)
  {
    for (int day = 0; day < time_periods; ++day)
    {
      if (kv.second[day] <= 0.0)
      {
        idle_penalty -= penalty;
      }
    }
  }

  // Combine final
  double final_score = weight_balance * cv_score +
                       weight_workload * (total_workload / 100.0) +
                       idle_penalty;

  return final_score;
}
// ParallelSimulation::ParallelSimulation() : Simulation(){};

ParallelSimulation::ParallelSimulation(
    const double _max_sim_time, const std::string &config_path
)
    : Simulation(_max_sim_time, config_path)
{
  this->initialize_event_handlers();
}

// Helper fuction para evaluar si todos las colas de salida están vacías
bool check_empty_out_queues(std::unordered_map<int, std::vector<Message>> queues
)
{
  for (auto const &[proc, queue] : queues)
    if (!queue.empty())
      return false;
  return true;
}

void ParallelSimulation::run()
{
  this->nprocs = bsp_nprocs();
  // Inicializamos la cola de salida
  for (bsp_size_t i = 0; i < this->nprocs; ++i)
    this->out_queue[i] = std::vector<Message>();

  this->monitor = new PostgresAggregatedMonitor();
  this->pid = bsp_pid();
  printf("Procesador %d/%d\n", pid, nprocs);

  // Leemos los archivos de entrada e inicializamos
  // objetos de contexto
  this->read_products();
  this->env->set_productos(this->productos);
  this->read_ferias();
  this->env->set_ferias(this->ferias);
  this->read_terrenos();
  auto mercado = new MercadoMayorista(this->env);
  this->mercado = mercado;
  printf("[INICIALIZANDO AGENTES]\n");
  this->initialize_agents(mercado);

  this->env->set_agricultores(this->agricultores);
  this->mercado->reset_index();

  std::map<int, bool> active_feria_dia;

  // Inicializamos los eventos del ambiente
  for (int i = 0; i < 7; ++i)
  {
    active_feria_dia.insert_or_assign(i, false);
  }

  for (const auto [feria_id, feria] : this->ferias)
  {
    for (const int dia : feria->get_dia_funcionamiento())
      active_feria_dia.insert_or_assign(dia, true);
  }

  for (const auto [dia, feria] : active_feria_dia)
  {
    if (!feria)
      continue;
    this->fel->insert_event(
        24.0 * dia, AGENT_TYPE::AMBIENTE, EVENTOS_AMBIENTE::INICIO_FERIA, 0,
        Message()
    );
  }

  double start, stop;
  bsp_sync();

  printf("[INICIANDO SIMULACION]\n");
  Event *current_event;

  // Para conteo de eventos
  std::map<std::string, int> agent_type_count_global = {
      {"CONSUMIDOR", 0}, {"FERIANTE", 0}, {"AGRICULTOR", 0}, {"AMBIENTE", 0}
  };
  std::string agent_type;
  std::map<std::string, int> event_type_count_global = {
      {"FERIANTE_COMPRA_MAYORISTA", 0},
      {"FERIANTE_VENTA_CONSUMIDOR", 0},
      {"FERIANTE_PROCESS_COMPRA_MAYORISTA", 0},
      {"FERIANTE_COMPRA_MAYORISTA", 0},
      {"CONSUMIDOR_BUSCAR_FERIANTE", 0},
      {"CONSUMIDOR_INIT_COMPRA_FERIANTE", 0},
      {"CONSUMIDOR_PROCESAR_COMPRA_FERIANTE", 0},
      {"CONSUMIDOR_COMPRA_FERIANTE", 0},
      {"AGRICULTOR_CULTIVO_TERRENO", 0},
      {"AGRICULTOR_COSECHA", 0},
      {"AGRICULTOR_VENTA_FERIANTE", 0},
      {"AGRICULTOR_INVENTARIO_VENCIDO", 0},
      {"AMBIENTE_INICIO_FERIA", 0},
      {"AMBIENTE_FIN_FERIA", 0},
      {"AMBIENTE_CALCULO_PRECIOS", 0},
      {"AMBIENTE_LIMPIEZA_MERCADO_MAYORISTA", 0}
  };

  std::map<std::string, double> event_type_time_global = {
      {"FERIANTE_COMPRA_MAYORISTA", 0.0},
      {"FERIANTE_VENTA_CONSUMIDOR", 0.0},
      {"FERIANTE_PROCESS_COMPRA_MAYORISTA", 0.0},
      {"FERIANTE_COMPRA_MAYORISTA", 0.0},
      {"CONSUMIDOR_BUSCAR_FERIANTE", 0.0},
      {"CONSUMIDOR_INIT_COMPRA_FERIANTE", 0.0},
      {"CONSUMIDOR_PROCESAR_COMPRA_FERIANTE", 0.0},
      {"CONSUMIDOR_COMPRA_FERIANTE", 0.0},
      {"AGRICULTOR_CULTIVO_TERRENO", 0.0},
      {"AGRICULTOR_COSECHA", 0.0},
      {"AGRICULTOR_VENTA_FERIANTE", 0.0},
      {"AGRICULTOR_INVENTARIO_VENCIDO", 0.0},
      {"AMBIENTE_INICIO_FERIA", 0},
      {"AMBIENTE_FIN_FERIA", 0},
      {"AMBIENTE_CALCULO_PRECIOS", 0},
      {"AMBIENTE_LIMPIEZA_MERCADO_MAYORISTA", 0}
  };
  int nsteps = 0;
  double window_size = 6;
  double next_window = this->fel->get_time() + window_size;
  long total_sync_time = 0;
  time_t t_init = time(NULL);
  SSTimeRecord time_record;
  SSEventRecord event_record;

  // Ciclo principal de simulación
  while (this->gvt <= this->max_sim_time && !fel->is_empty())
  {

    event_record.ss_number = nsteps;
    event_record.proc_id = this->pid;
    time_record.ss_number = nsteps;
    time_record.proc_id = this->pid;

    double ss_init = bsp_time();
    while (this->fel->get_time() <= next_window)
    {

      current_event = fel->next_event();
      agent_type = agent_type_to_agent.at(current_event->get_type());
      ++event_record.agent_type_count[agent_type];
      ++event_record.event_type_count[event_type_to_type
                                          .at(current_event->get_process())];
      // Para mostrar algo por consola
      if ((current_event->event_id % 10'000'000) == 0)
        printf(
            "[PROC %d/%d] - SIM TIME: %lf\tEVENT ID: %d\n", this->pid,
            this->nprocs, fel->get_time(), current_event->event_id
        );
      this->route_event(current_event);
      this->fel->event_pool.release(current_event);
      // delete current_event;
    }
    double end_ss_exec = bsp_time();
    // Aumentamos el contador de super pasos
    nsteps++;

    // Actualizamos la próxima ventana
    next_window += window_size;

    // Mandamos mensajes de salida
    for (auto &[proc, messages] : this->out_queue)
    {
      MessageSerializer::send(std::move(messages), proc);
      messages.clear();
    }
    // auto start = std::chrono::high_resolution_clock::now();
    start = bsp_time();
    // Enviamos y recibimos todos los mensajes
    bsp_sync();
    stop = bsp_time();

    total_sync_time += (stop - start);
    time_record.exec_time = (end_ss_exec - ss_init);
    time_record.sync_time = (stop - start);

    std::vector<Message> incoming = MessageSerializer::receive();
    // printf("INCOMING SIZE: %ld\n", incoming.size());
    for (auto msg : incoming)
    {
      this->message_handler(msg);
    }

    // printf("[PROC %d] || size fel %ld\n", this->pid, this->fel->get_size());
    // Si se cumple la frecuencia, actualizamos
    if ((nsteps % this->GVT_CALCULATION_FREQ) == 0)
    {
      this->update_gvt();
      printf(
          "[PROC %d] || GVT: %lf - LVT: %lf - SS: %d \n", this->pid, this->gvt,
          this->fel->get_time(), nsteps
      );
    }

    this->monitor->add_event_record(event_record);
    this->monitor->add_time_record(time_record);
  }
  time_t t_end = time(NULL);

  bsp_sync();
  this->monitor->write_duration(t_end - t_init);
  this->monitor->write_results();
  printf(
      "[PROC %d] WALL_TIME: %ld\tBSP_TIME: %lf\tSYNC_TIME: %ld[microseconds]\n",
      this->pid, t_end - t_init, bsp_time(), total_sync_time
  );
  printf("[PROD %d] Last Event ID: %d\n", this->pid, current_event->event_id);
}

void ParallelSimulation::route_event(Event *e)
{

  // printf(
  //     "caller_id: %d caller_type: %d process: %d \n", e->get_caller_id(),
  //     e->get_type(), e->get_process()
  // );
  // Manejamos cada evento según su tipo
  /**switch (e->get_type())
  {
  case AGENT_TYPE::AMBIENTE:
  {
    this->env->process_event(e);
    return;
  }
  case AGENT_TYPE::AGRICULTOR:
  {
    switch (e->get_process())
    {
    case EVENTOS_AGRICULTOR::COSECHA:
    { // TODO: Revisar caso con matriz de inventario en MercadoMayorista
      // Revisamos el terreno **después** de elegir el producto
      // y mandamos al agricultor al procesador correspondiente
      Message msg = e->get_message();
      msg.insert(MESSAGE_KEYS::ORIGIN_PID, (double)this->pid);
      int new_prod_id = this->agricultor_arr.at(e->get_caller_id())
                            ->get_terreno()
                            ->get_producto();

      int target_proc = this->proc_per_prod.at(new_prod_id);
      if (target_proc != this->pid)
        this->out_queue[target_proc].push_back(e->get_message());
      else
      {
        Agricultor *agr = this->agricultor_arr[e->get_caller_id()];
        agr->process_event(e);
      }
      return;
    }
    case EVENTOS_AGRICULTOR::VENTA_FERIANTE:
    {
      // En el caso de la VENTA_FERIANTE, i.e. donde un feriante
      // quiere comprar un prod_id específico a un agricultor,
      // tenemos dos casos.
      //
      // En el primero, el producto que se quiere comprar existe en el
      // procesador. Aquí simplemente se procesa.
      //
      // En el segundo caso, el producto objetivo existe en otro procesador.
      // Así que lo encolamos para salida y descartamos el evento sin procesar
      //
      Message msg = e->get_message();
      int prod_id = (int)msg.find(MESSAGE_KEYS::PROD_ID);
      int target_proc = this->proc_per_prod.at(prod_id);

      if (target_proc == this->pid)
      {
        double amount = msg.find(MESSAGE_KEYS::AMOUNT);
        // Atendemos la compra y encolamos la respuesta
        std::vector<int> agros_id =
            this->mercado->get_agricultor_por_prod(prod_id);

        Agricultor *agro;
        for (const auto &agr_id : agros_id)
        {
          agro = this->agricultor_arr[agr_id];
          Inventario inv = agro->get_inventory_by_id(prod_id);
          // Si el agricultor no tiene inventario válido o cantidad insuficiente, seguimos buscando
          if (!inv.is_valid_inventory() || inv.get_quantity() < amount)
            continue;

          // Si efectivamente tiene, dejamos que procese la compra
          agro->process_event(e);
          return;
        }
      }
      else
      {
        // Si el target_proc no es aquí, significa que
        // el procesador de origen somos nosotros y estamos mandando a comprar
        // afuera Así que necesitamos setear el ORIGIN_PID para poder manejarlo
        msg.insert(MESSAGE_KEYS::ORIGIN_PID, (double)this->pid);
        this->out_queue[target_proc].push_back(msg);
      }
      return;
    }
    default:
    {
      // Caso por defecto, tratamos de traer al agricultor y
      // procesar el evento
      Agricultor *agr = this->agricultor_arr.at(e->get_caller_id());

      agr->process_event(e);
      return;
    }
    }
  }
  case AGENT_TYPE::FERIANTE:
  {
    // En el caso de comprar a un agricultor,
    // hay que verificar el prod_id para saber
    // a qué procesador mandarlo
    switch (e->get_process())
    {
    case EVENTOS_FERIANTE::PROCESS_COMPRA_MAYORISTA:
    {
      Message msg = e->get_message();
      int origin_pid = (int)msg.find(MESSAGE_KEYS::ORIGIN_PID);
      // Si el ORIGIN_PID es distinto a nuestro pid actual
      // significa que la solicitud original de compra viene desde otro
      // procesador Así que lo insertamos en la cola de salida.
      //
      // Si efectivamente calzan, simplemente procesamos evento
      if (origin_pid != this->pid && origin_pid != -1)
      {
        int prod_id = (int)msg.find(MESSAGE_KEYS::PROD_ID);
        int target_proc = this->proc_per_prod.at(prod_id);
        this->out_queue[target_proc].push_back(msg); // TODO: Revisar
      }
      else
      {
        int agent_id = (int)msg.find(MESSAGE_KEYS::AGENT_ID);
        Feriante *fer = this->feriante_arr.at(agent_id);
        fer->process_event(e);
      }

      return;
    }
    default:
    {
      int feriante_id = e->get_caller_id();

      this->feriante_arr.at(feriante_id)->process_event(e);
      break;
    }
    }
  }
  // Consumidor siempre intercambiará eventos con otros agentes locales al
  // procesador Así que siempre procesamos nomas
  case AGENT_TYPE::CONSUMIDOR:
  {
    Agent *caller = e->get_caller_ptr();
    caller->process_event(e);
  }
  }**/
  event_handlers.handle(this, e);
}

void ParallelSimulation::message_handler(Message &msg)
{
  int agent_type = (int)msg.find(MESSAGE_KEYS::AGENT_TYPE);
  int process = (int)msg.find(MESSAGE_KEYS::PROCESS);
  int agent_id;

  // Si el evento es PROCESS_COMPRA_MAYORISTA, significa que es
  // una respuesta de un procesador foráneo. El id del agente
  // está  en la llave BUYER_ID
  if (agent_type == AGENT_TYPE::FERIANTE &&
      process == EVENTOS_FERIANTE::PROCESS_COMPRA_MAYORISTA)
  {
    agent_id = msg.find(MESSAGE_KEYS::BUYER_ID);
  }
  else
    agent_id = msg.find(MESSAGE_KEYS::AGENT_ID);
  // printf(
  //     "HANDLER: caller_id: %d caller_type: %d process: %d \n", agent_id,
  //     agent_type, process
  // );
  this->fel->insert_event(0.0, agent_type, process, agent_id, msg);
}

void ParallelSimulation::read_ferias()
{

  json config = SimConfig::get_instance()->get_config();

  std::unordered_map<int, Feria *> ferias;
  /** Para consumidores y feriantes*/
  std::ifstream ferias_f(config["ferias_file"].get<std::string>());
  json ferias_json = json::parse(ferias_f);
  int cant_ferias = 0;
  int cant_puestos = 0;

  printf("Creando ferias y consumidores\n");
  for (auto feria : ferias_json)
  {
    std::vector<int> dias_funcionamiento =
        feria["dias"].get<std::vector<int>>();

    // Actualizamos la cantidad de ferias
    ++cant_ferias;
    cant_puestos += feria["cantidad_puestos"].get<int>();
    auto fer = new Feria(
        dias_funcionamiento, feria["cantidad_puestos"].get<int>(), this->env,
        this->fel
    );
    ferias.insert({fer->get_id(), fer});
  }
  std::vector<int> mis_ferias_ids;
  // Particionamos las ferias en la cantidad de procesadores
  if (this->pid == 0)
  {
    printf("[PROC: %d] Particionando productos por proc\n", this->pid);
    SimmulatedAnnealingPartitioner<Feria> feria_partitioner(
        ferias, feria_objective_function
    );

    std::unordered_map<int, std::vector<Feria *>> ferias_per_proc;
    ferias_per_proc = feria_partitioner.partition_items(
        this->nprocs, 0, 1, 2000, 1e-5, 0.99, true
    );

    // Generamos un arreglo equivalente, pero solo con los IDs
    // (mejor para el envío de mensajes)

    std::unordered_map<int, std::vector<int>> ferias_id_per_proc;
    // std::cout << "Ferias per proc: \n";
    for (auto const &[proc, ferias] : ferias_per_proc)
    {
      ferias_id_per_proc[proc] = std::vector<int>(ferias.size(), 0);
      //   std::cout << "PROC: " << proc << "\t";
      for (size_t i = 0; i < ferias.size(); ++i)
      {
        //     std::cout << ferias[i]->get_id() << " ";
        ferias_id_per_proc[proc].at(i) = ferias[i]->get_id();
      }
      //   std::cout << std::endl;
    }

    // Mandamos los mensajes
    int tag = 0;
    for (int i = 0; i < this->nprocs; ++i)
      bsp_send(
          i, &tag, ferias_id_per_proc[i].data(),
          ferias_id_per_proc[i].size() * sizeof(int)
      );
  }
  bsp_sync();

  printf("[PROC: %d] Recibiendo productos particionados\n", this->pid);
  int num_messages;
  int accum_bytes;

  bsp_qsize(&num_messages, &accum_bytes);
  mis_ferias_ids.resize(accum_bytes / sizeof(int));
  bsp_move(mis_ferias_ids.data(), accum_bytes);

  bsp_sync();
  // En este punto, tenemos todas las ferias en todos los procesadores,
  // y tenemos el vector con las ferias asignadas a este procesador
  //
  // Ahora solamente guardamos las ferias que nos competen

  std::unordered_map<int, Feria *> mis_ferias;
  for (size_t i = 0; i < mis_ferias_ids.size(); ++i)
  {
    mis_ferias.insert({i, ferias.at(mis_ferias_ids[i])});
    this->feria_arr.push_back(ferias.at(mis_ferias_ids[i]));
  }

  this->ferias = mis_ferias;

  // Para debuggeo
  printf(
      "[PROC: %d/%d] - Num ferias: %d & Num feriantes: %d\n", this->pid,
      this->nprocs, cant_ferias, cant_puestos
  );
}

void ParallelSimulation::read_products()
{

  json config = SimConfig::get_instance()->get_config();
  /** Para productos */
  std::ifstream prods_f(config["prod_file"].get<std::string>());
  json prods = json::parse(prods_f);
  printf("Inicializando productos\n");
  std::unordered_map<int, Producto *> prod_map;
  for (auto it : prods)
  {
    std::vector<int> meses_siembra =
        it["meses_siembra"].get<std::vector<int>>();
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

  printf(
      "[PROC: %d/%d] - Aki vamo leyendo productos a lo loco\n", this->pid,
      this->nprocs
  );
  int num_mensajes;
  int num_bytes;
  int tag;
  int status;
  // Particionamos los productos por procesador
  if (this->pid == 0)
  {
    // auto product_partitioner = ProductPartioner(this->productos);

    SimmulatedAnnealingPartitioner<Producto> product_partitioner(
        this->productos, product_objective_function
    );
    std::unordered_map<int, std::vector<Producto *>> prods_por_proc;
    prods_por_proc = product_partitioner.partition_items(
        this->nprocs, 0, 2, 2000, 1e-5, 0.98, true
    );
    this->prods_per_proc = prods_por_proc;
    std::cout << "Prods por proc:\n";
    for (auto const &[proc, prods] : prods_por_proc)
    {
      std::cout << "PROC: " << proc << "\t";
      for (auto const prod : prods)
      {
        std::cout << prod->get_id() << " (" << prod->get_nombre() << "), ";
      }
      std::cout << "\n";
    }
    // Ahora que tenemos qué productos van en qué procesador,
    // hay que enviar la info a cada nodo.
    //
    // Son las 1:15 AM, estoy en reu desde
    // las 8:30 AM del día anterior, y esta es la mejor solución que se me
    // ocurre.
    //
    // La idea es burdamente simple:
    // un arreglo `arr` de largo `this->nprocs`, donde `arr[i]` es
    // el procesador al que pertenece el producto  con id `i`

    int *prods_proc_arr = new int[this->productos.size()];
    // Poblamos y mandamos la info a cada proc
    for (auto const &[proc, prods] : prods_por_proc)
    {
      for (auto const &id : prods)
        prods_proc_arr[id->get_id()] = proc;
    }
    for (int target_proc = 1; target_proc < this->nprocs; ++target_proc)
      bsp_send(
          target_proc, &tag, prods_proc_arr,
          sizeof(int) * this->productos.size()
      );
  }
  bsp_sync();

  // Recibimos el mensaje
  if (this->pid != 0)
  {
    bsp_qsize(&num_mensajes, &num_bytes);

    // Parseamos el tag
    bsp_get_tag(&status, &tag);

    if (status == -1)
      bsp_abort("Error al recibir particionamiento de productos.\n");
    int *prods_proc_arr = new int[this->productos.size()];
    bsp_move(prods_proc_arr, status);

    // Construimos la estructura de datos adecuada
    for (int i = 0; i < this->nprocs; ++i)
    {
      this->prods_per_proc.insert({i, std::vector<Producto *>()});
    }

    for (size_t i = 0; i < this->productos.size(); ++i)
    {
      this->prods_per_proc.at(prods_proc_arr[i])
          .push_back(this->productos.at(i));
    }

    delete[] prods_proc_arr;
  }

  bsp_sync();

  // Generamos el arreglo de procs por prod y el mapeo
  // de últimp vendedor por producto
  this->proc_per_prod = std::vector<int>(this->productos.size());
  for (auto const &[proc, prods] : this->prods_per_proc)
  {
    for (auto const prod : prods)
    {
      this->proc_per_prod[prod->get_id()] = proc;
    }
  }

  for (auto const &prod : this->productos)
  {
    this->last_seller_by_prod[prod.first] = 0;
  }
  std::string output = "[PROC: " + std::to_string(this->pid) + "/" +
                       std::to_string(this->nprocs) + "] ";

  for (auto const &[proc, prods] : this->prods_per_proc)
  {
    output = output + "PROC_ID: " + std::to_string(proc) + "\t";
    for (auto const &prod : prods)
      output = output + std::to_string(prod->get_id()) + " ";
    output = output + "\n";
  }
  printf("%s\n", output.c_str());
  bsp_sync();
}

void ParallelSimulation::read_terrenos()
{
  int num_mensajes;
  int num_bytes;
  int tag;
  int status;
  json config = SimConfig::get_instance()->get_config();
  int *prods_por_terreno_arr = nullptr;
  /** Vamos por los terrenos*/
  std::ifstream terr_f(config["terrenos_file"].get<std::string>());
  json terrenos_json = json::parse(terr_f);
  printf("Inicializando terrenos\n");
  std::vector<int> prods_por_terreno;

  if (this->pid == 0)
  {
    // PROC 0 Asigna los terrenos, genera sus propias instancias y envía
    // la asignación al resto de los procesadores
    for (auto terreno : terrenos_json)
    {
      std::random_device rd;
      std::mt19937 gen(rd());
      std::uniform_int_distribution<> d(0, this->productos.size() - 1);
      int prod;
      prod = d(gen);
      prods_por_terreno.push_back(prod);
      auto terr = new Terreno(
          terreno["cod_comuna"].get<int>(), terreno["area_ha"].get<double>(),
          terreno["comuna"].get<std::string>(), prod
      );

      this->terrenos.insert({terr->get_id(), terr});
    }
    // Generamos el arreglo estático para mandar por la red
    prods_por_terreno_arr = new int[this->terrenos.size()];
    for (size_t i = 0; i < prods_por_terreno.size(); ++i)
      prods_por_terreno_arr[i] = prods_por_terreno[i];

    // Encolamos el mensaje para todo el resto de los procesadores
    for (int target_proc = 1; target_proc < this->nprocs; ++target_proc)
      bsp_send(
          target_proc, &tag, prods_por_terreno_arr,
          sizeof(int) * this->terrenos.size()
      );
    printf("NUM TERRENOS: %d\n", (int)this->terrenos.size());
  }
  bsp_sync();

  // Siguiente paso, parseamos los mensajes e instanciamos los terrenos
  if (this->pid != 0)
  {
    // Parseamos la cantidad de mensajes recibidos
    bsp_qsize(&num_mensajes, &num_bytes);

    // Parseamos el tag
    bsp_get_tag(&status, &tag);
    if (status == -1)
      bsp_abort("Error al recibir distribuición de terrenos.\n");

    // Aquí, status tiene el largo en bytes del payload en la cola
    prods_por_terreno_arr = new int[status / sizeof(int)];
    printf("NUM TERRENOS RECIBIDOS %lu\n", status / sizeof(int));
    bsp_move(prods_por_terreno_arr, status);
    int i = 0;
    for (auto terreno : terrenos_json)
    {
      auto terr = new Terreno(
          terreno["cod_comuna"].get<int>(), terreno["area_ha"].get<double>(),
          terreno["comuna"].get<std::string>(), prods_por_terreno_arr[i]
      );
      ++i;
      this->terrenos.insert({terr->get_id(), terr});
    }
  }
  delete[] prods_por_terreno_arr;

  bsp_sync();
  std::string output = "[PROC: " + std::to_string(this->pid) + "/" +
                       std::to_string(this->nprocs) + "] ";
  for (auto const &[id, terr] : this->terrenos)
  {
    output = output + "TERR_ID: " + std::to_string(id) + "-" +
             "PROD_ID: " + std::to_string(terr->get_producto()) + "\t";
  }
  bsp_sync();
}

void ParallelSimulation::initialize_agents(MercadoMayorista *_mer)
{
  json config = SimConfig::get_instance()->get_config();

  printf("[INICIALIZANDO AGENTES]\t");
  printf("Feriantes -\t");
  // Inicializamos feriantes y consumidores
  for (auto feria : this->ferias)
  {
    // std::cout << "cantidad de terrenos: " << this->terrenos.size() << "\n";
    std::map<int, Feriante *> current_feriantes;
    auto feriante_factory =
        FerianteFactory(this->fel, this->env, this->monitor, _mer);
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

    int consumidores_por_puesto =
        config["consumidor_feriante_ratio"].get<int>();
    std::string consumer_type = config["tipo_consumidor"].get<std::string>();
    int cantidad_consumidores =
        consumidores_por_puesto * feria.second->get_num_feriantes();

    auto cons_factory = ConsumidorFactory(this->fel, this->env, this->monitor);

    for (int i = 0; i < cantidad_consumidores; ++i)
    {
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

  for (auto terr : this->terrenos)
  {
    agr = agricultor_factory.create_agricultor(agricultor_type, terr.second);
    this->agricultores.insert({agr->get_id(), agr});
    this->agricultor_arr.push_back(agr);
    this->agricultor_por_id_relativo.insert({agr->get_agricultor_id(), agr});
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
}

void ParallelSimulation::update_gvt()
{
  int tag;
  int status;
  int num_msgs;
  int accum_bytes;
  double lvt = this->fel->get_time();

  // Enviamos nuestros LVT a todos los procs
  for (int i = 0; i < this->nprocs; ++i)
    bsp_send(i, &tag, &lvt, sizeof(double));

  // Sincronizamos y buscamos el mínimo
  bsp_sync();

  bsp_qsize(&num_msgs, &accum_bytes);
  // Recibimos cada mensaje y comparamos
  for (int i = 0; i < num_msgs; ++i)
  {
    double foreign_lvt;
    bsp_get_tag(&status, &tag);
    bsp_move(&foreign_lvt, status);
    if (foreign_lvt < lvt)
      lvt = foreign_lvt;
  }
  printf("NUEVO GVT: %lf\n", this->gvt);
  this->gvt = lvt;
}

void ParallelSimulation::initialize_event_handlers()
{
  // AMBIENTE Events
  event_handlers.register_handler(
      AGENT_TYPE::AMBIENTE, EVENTOS_AMBIENTE::INICIO_FERIA,
      [](ParallelSimulation *sim, Event *e)
      {
        // Environment events are always processed locally
        sim->env->process_event(e);
      }
  );

  event_handlers.register_handler(
      AGENT_TYPE::AMBIENTE, EVENTOS_AMBIENTE::FIN_FERIA,
      [](ParallelSimulation *sim, Event *e) { sim->env->process_event(e); }
  );

  event_handlers.register_handler(
      AGENT_TYPE::AMBIENTE, EVENTOS_AMBIENTE::CALCULO_PRECIOS,
      [](ParallelSimulation *sim, Event *e) { sim->env->process_event(e); }
  );

  event_handlers.register_handler(
      AGENT_TYPE::AMBIENTE, EVENTOS_AMBIENTE::LIMPIEZA_MERCADO_MAYORISTA,
      [](ParallelSimulation *sim, Event *e) { sim->env->process_event(e); }
  );

  // AGRICULTOR Events
  event_handlers.register_handler(
      AGENT_TYPE::AGRICULTOR, EVENTOS_AGRICULTOR::CULTIVO_TERRENO,
      [](ParallelSimulation *sim, Event *e)
      {
        // Standard agricultor event - process locally
        Agricultor *agr = sim->agricultor_arr.at(e->get_caller_id());
        agr->process_event(e);
      }
  );

  event_handlers.register_handler(
      AGENT_TYPE::AGRICULTOR, EVENTOS_AGRICULTOR::COSECHA,
      [](ParallelSimulation *sim, Event *e)
      {
        // Check if we need to route to a different processor
        Message msg = e->get_message();
        msg.insert(MESSAGE_KEYS::ORIGIN_PID, (double)sim->get_pid());
        int new_prod_id = sim->agricultor_arr.at(e->get_caller_id())
                              ->get_terreno()
                              ->get_producto();
        msg.insert(MESSAGE_KEYS::AGENT_ID, e->get_caller_id());
        msg.insert(MESSAGE_KEYS::AGENT_TYPE, AGENT_TYPE::AGRICULTOR);
        msg.insert(MESSAGE_KEYS::PROCESS, EVENTOS_AGRICULTOR::COSECHA);
        int target_proc = sim->proc_per_prod.at(new_prod_id);
        if (target_proc != sim->pid)
        {
          // Send to the processor that handles this product
          sim->out_queue[target_proc].push_back(msg);
        }
        else
        {
          // Process locally
          Agricultor *agr = sim->agricultor_arr[e->get_caller_id()];
          agr->process_event(e);
        }
      }
  );

  event_handlers.register_handler(
      AGENT_TYPE::AGRICULTOR, EVENTOS_AGRICULTOR::VENTA_FERIANTE,
      [](ParallelSimulation *sim, Event *e)
      {
        // Handle feriante purchasing from agricultor
        Message msg = e->get_message();
        int prod_id = (int)msg.find(MESSAGE_KEYS::PROD_ID);
        
        // Get product name for debugging
        std::string prod_name = "unknown";
        try {
          Producto* producto = sim->productos.at(prod_id);
          if (producto) {
            prod_name = producto->get_nombre();
          }
        } catch (const std::exception& e) {
          // Handle error silently
        }
        
        // Debug problematic products
        bool is_problematic = (prod_name == "Ajo" || prod_name == "Alcachofa" || 
                              prod_name == "Arveja Verde" || prod_name == "Choclo" || 
                              prod_name == "Frutilla" || prod_name == "Haba" || 
                              prod_name == "Pepino ensalada" || prod_name == "Tomate");
        
        if (is_problematic) {
          printf("[DEBUG] Processing VENTA_FERIANTE for problematic product %d (%s)\n", 
                 prod_id, prod_name.c_str());
        }
        
        int target_proc = sim->proc_per_prod.at(prod_id);

        if (target_proc == sim->pid)
        {
          // Local processing
          double amount = msg.find(MESSAGE_KEYS::AMOUNT);
          // Get available agricultores for this product
          std::vector<int> agros_id =
              sim->mercado->get_agricultor_por_prod(prod_id);
              
          if (is_problematic) {
            printf("[DEBUG] Product %d (%s) - Found %zu agricultores with inventory\n", 
                   prod_id, prod_name.c_str(), agros_id.size());
          }

          // Find an agricultor with sufficient inventory
          Agricultor *agro;
          bool found_valid_inventory = false;
          
          for (const auto &agr_id : agros_id)
          {
            agro = sim->agricultor_arr[agr_id];
            Inventario inv = agro->get_inventory_by_id(prod_id);
            double quantity = inv.get_quantity();
            
            if (is_problematic) {
              printf("[DEBUG] Product %d (%s) - Agricultor %d inventory: valid=%d, quantity=%.2f, needed=%.2f\n",
                     prod_id, prod_name.c_str(), agr_id, inv.is_valid_inventory(), quantity, amount);
            }
            
            // Skip if inventory is invalid or insufficient
            if (!inv.is_valid_inventory() || quantity < amount)
              continue;
              
            // Found a suitable agricultor, process the sale
            if (is_problematic) {
              printf("[DEBUG] Product %d (%s) - Found valid inventory, processing sale\n", 
                     prod_id, prod_name.c_str());
            }
            
            found_valid_inventory = true;
            agro->process_event(e);
            return;
          }
          
          if (is_problematic && !found_valid_inventory) {
            printf("[DEBUG] Product %d (%s) - NO VALID INVENTORY FOUND\n", 
                   prod_id, prod_name.c_str());
          }
          
          return;
        }
        else
        {
          // Route to appropriate processor
          msg.insert(MESSAGE_KEYS::ORIGIN_PID, (double)sim->pid);
          
          if (is_problematic) {
            printf("[DEBUG] Product %d (%s) - Routing to processor %d from %d\n", 
                   prod_id, prod_name.c_str(), target_proc, sim->pid);
          }
          
          auto origin_pid = msg.find(MESSAGE_KEYS::ORIGIN_PID);
          msg.insert(MESSAGE_KEYS::AGENT_TYPE, AGENT_TYPE::FERIANTE);
          msg.insert(
              MESSAGE_KEYS::PROCESS, EVENTOS_FERIANTE::PROCESS_COMPRA_MAYORISTA
          );
          sim->out_queue[target_proc].push_back(msg);
        }
      }
  );

  event_handlers.register_handler(
      AGENT_TYPE::AGRICULTOR, EVENTOS_AGRICULTOR::INVENTARIO_VENCIDO,
      [](ParallelSimulation *sim, Event *e)
      {
        // Standard event - process locally
        Agricultor *agr = sim->agricultor_arr.at(e->get_caller_id());
        agr->process_event(e);
      }
  );

  // FERIANTE Events
  event_handlers.register_handler(
      AGENT_TYPE::FERIANTE, EVENTOS_FERIANTE::COMPRA_MAYORISTA,
      [](ParallelSimulation *sim, Event *e)
      {
        Message msg = e->get_message();
        msg.insert(MESSAGE_KEYS::ORIGIN_PID, sim->pid);
        // Standard feriante event - process locally
        int feriante_id = e->get_caller_id();
        sim->feriante_arr.at(feriante_id)->process_event(e);
      }
  );

  event_handlers.register_handler(
      AGENT_TYPE::FERIANTE, EVENTOS_FERIANTE::VENTA_CONSUMIDOR,
      [](ParallelSimulation *sim, Event *e)
      {
        // Standard feriante event - process locally
        int feriante_id = e->get_caller_id();
        sim->feriante_arr.at(feriante_id)->process_event(e);
      }
  );

  event_handlers.register_handler(
      AGENT_TYPE::FERIANTE, EVENTOS_FERIANTE::PROCESS_COMPRA_MAYORISTA,
      [](ParallelSimulation *sim, Event *e)
      {
        // Complex case - may need routing to another processor
        Message msg = e->get_message();
        int origin_pid = (int)msg.find(MESSAGE_KEYS::ORIGIN_PID);
        // Check if this message is from another processor
        if (origin_pid != sim->pid && origin_pid != -1)
        {
          int prod_id = (int)msg.find(MESSAGE_KEYS::PROD_ID);
          auto seller = (int)msg.find(MESSAGE_KEYS::SELLER_ID);
          auto buery = (int)msg.find(MESSAGE_KEYS::BUYER_ID);
          auto error = msg.find(MESSAGE_KEYS::ERROR);
          int target_proc = sim->proc_per_prod.at(prod_id);
          // if (origin_pid < 0)
          // printf(
          //     "ORIGIN PID EN PROCESS_COMPRA_MAYORISTA: %lf BUYER_ID: %d "
          //     "SELLER_ID: %d PROD_ID: %d\n",
          //    msg.find(MESSAGE_KEYS::ORIGIN_PID), buery, seller, prod_id
          //  );
          sim->out_queue[origin_pid].push_back(msg);
        }
        else
        {
          // Process locally
          int agent_id = (int)msg.find(MESSAGE_KEYS::AGENT_ID);
          auto seller = (int)msg.find(MESSAGE_KEYS::SELLER_ID);
          auto error = msg.find(MESSAGE_KEYS::ERROR);
          // printf(
          //     "Procesando respuesta exitosa de compra de feriante! "
          //     "SELLER_ID: %d ERROR: %lf\n",
          //     seller, error
          //);
          Feriante *fer = sim->feriante_arr.at(agent_id);
          fer->process_event(e);
        }
      }
  );

  // CONSUMIDOR Events
  event_handlers.register_handler(
      AGENT_TYPE::CONSUMIDOR, EVENTOS_CONSUMIDOR::BUSCAR_FERIANTE,
      [](ParallelSimulation *sim, Event *e)
      {
        // Consumidor events are always local
        Agent *caller = e->get_caller_ptr();
        caller->process_event(e);
      }
  );

  event_handlers.register_handler(
      AGENT_TYPE::CONSUMIDOR, EVENTOS_CONSUMIDOR::INIT_COMPRA_FERIANTE,
      [](ParallelSimulation *sim, Event *e)
      {
        Agent *caller = e->get_caller_ptr();
        caller->process_event(e);
      }
  );

  event_handlers.register_handler(
      AGENT_TYPE::CONSUMIDOR, EVENTOS_CONSUMIDOR::PROCESAR_COMPRA_FERIANTE,
      [](ParallelSimulation *sim, Event *e)
      {
        Agent *caller = e->get_caller_ptr();
        caller->process_event(e);
      }
  );

  event_handlers.register_handler(
      AGENT_TYPE::CONSUMIDOR, EVENTOS_CONSUMIDOR::COMPRA_FERIANTE,
      [](ParallelSimulation *sim, Event *e)
      {
        Agent *caller = e->get_caller_ptr();
        caller->process_event(e);
      }
  );

  // Default handler for any unregistered event types
  event_handlers.set_default_handler(
      [](ParallelSimulation *sim, Event *e)
      {
        int agent_type = e->get_type();
        int caller_id = e->get_caller_id();

        // Fall back to the standard event processing based on agent type
        switch (agent_type)
        {
        case AGENT_TYPE::AMBIENTE:
          sim->env->process_event(e);
          break;

        case AGENT_TYPE::AGRICULTOR:
        {
          if (caller_id >= 0 && caller_id < sim->agricultor_arr.size())
          {
            Agricultor *agr = sim->agricultor_arr.at(caller_id);
            agr->process_event(e);
          }
          else
          {
            printf("Warning: Invalid agricultor ID %d\n", caller_id);
          }
          break;
        }

        case AGENT_TYPE::FERIANTE:
        {
          if (caller_id >= 0 && caller_id < sim->feriante_arr.size())
          {
            Feriante *fer = sim->feriante_arr.at(caller_id);
            fer->process_event(e);
          }
          else
          {
            printf("Warning: Invalid feriante ID %d\n", caller_id);
          }
          break;
        }

        case AGENT_TYPE::CONSUMIDOR:
        {
          // Try to use caller_ptr first for safety
          Agent *caller = e->get_caller_ptr();
          if (caller)
          {
            caller->process_event(e);
          }
          else if (caller_id >= 0 && caller_id < sim->consumidores_arr.size())
          {
            Consumidor *con = sim->consumidores_arr.at(caller_id);
            con->process_event(e);
          }
          else
          {
            printf("Warning: Invalid consumidor ID %d\n", caller_id);
          }
          break;
        }

        default:
          printf("Warning: Unknown agent type %d\n", agent_type);
          break;
        }
      }
  );
}

ParallelSimulation::~ParallelSimulation() { printf("Eliminando...\n"); }
