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

// Función de aiuda para cálculo de función objetivo
// del particionador de productos
double product_objective_function(
    const std::unordered_map<int, Producto *> &items,
    const std::vector<int> &sol
)
{
  size_t num_items = items.size();
  const double penalty = 0.1;
  std::unordered_map<int, std::vector<double>> items_per_month_per_proc;

  // Initialize each partition with a 12-month vector (all months set to 0)
  for (size_t i = 0; i < num_items; ++i)
  {
    if (items_per_month_per_proc.find(sol[i]) == items_per_month_per_proc.end())
    {
      items_per_month_per_proc[sol[i]] = std::vector<double>(12, 0.0);
    }

    Producto *item = items.at(i);
    std::vector<int> meses_venta = item->get_meses_venta();

    // For each month the product is sold, increase the month's count
    for (const auto &month : meses_venta)
    {
      items_per_month_per_proc[sol[i]][month] +=
          item->get_probabilidad_consumo();
    }
  }

  double total_score = 0.0;

  // For each partition, compute the partition score
  for (const auto &[proc, items_per_month] : items_per_month_per_proc)
  {
    double partition_score = 0.0;

    // Calculate score for each month, applying penalty for months without
    // activity
    for (const auto &month_value : items_per_month)
    {
      if (month_value > 0)
      {
        partition_score += month_value; // Add value for months with sales
      }
      else
      {
        partition_score -= penalty; // Apply penalty for months without sales
      }
    }

    // Add the partition score to the total score
    total_score += partition_score;
  }

  return total_score; // Return the overall total score
  /**
  std::vector<double> centroid(12, 0.0);

  for (size_t i = 0; i < num_items; ++i)
  {
    std::vector<double> items_per_month;
    try
    {
      items_per_month = items_per_month_per_proc.at(sol[i]);
    }
    catch (std::out_of_range &e)
    {
      items_per_month = std::vector<double>(12, 0.0);
    }

    Producto *item = items.at(i);
    std::vector<int> meses_venta = item->get_meses_venta();

    for (const auto &month : meses_venta)
    {
      items_per_month[month]++;
    }

    items_per_month_per_proc[sol[i]] = items_per_month;

    for (int j = 0; j < 12; ++j)
      centroid[j] += items_per_month[j];
  }

  for (int i = 0; i < 12; ++i)
    centroid[i] /= (double)items_per_month_per_proc.size();

  double result = 0.0;
  for (const auto &[item, items_per_month] : items_per_month_per_proc)
  {
    for (int i = 0; i < 12; ++i)
      result += abs(centroid[i] - items_per_month[i]);
  }
  return result; **/
}

// Función de aiuda, pero para repartir ferias
double feria_objective_function(
    const std::unordered_map<int, Feria *> &items, const std::vector<int> &sol
)
{
  const int penalty = 200;
  size_t num_ferias = items.size();

  std::unordered_map<int, std::vector<double>>
      ferias_per_proc; // Partitions of ferias

  // Initialize partitions with 7-day vectors (initialized to 0)
  for (size_t i = 0; i < num_ferias; ++i)
  {
    if (ferias_per_proc.find(sol[i]) == ferias_per_proc.end())
    {
      ferias_per_proc[sol[i]] = std::vector<double>(7, 0.0);
    }

    Feria *feria = items.at(i);
    std::vector<int> dias_funcionamiento = feria->get_dia_funcionamiento();

    // Increment counts in the 7-day vector based on dias_funcionamiento
    for (const auto &dia : dias_funcionamiento)
    {
      ferias_per_proc[sol[i]][dia] += feria->get_num_feriantes();
    }
  }

  std::vector<double> scores;

  // For each partition (mapped by processor or partition ID)
  for (const auto &[proc, ferias_per_day] : ferias_per_proc)
  {
    double score = 0.0;

    // Compute score for this partition, applying penalties where necessary
    for (const auto &dia_value : ferias_per_day)
    {
      if (dia_value > 0)
      {
        score += dia_value; // Add value for days with ferias
      }
      else
      {
        score -= penalty; // Apply penalty for days with no ferias
      }
    }

    scores.push_back(score);
  }

  // Return the minimum score among all partitions
  return *std::min_element(scores.begin(), scores.end());
  // // En este caso, nos interesa evaluar por días de la semana
  // std::unordered_map<int, std::vector<double>> ferias_per_day_per_proc;
  // std::vector<double> centroid(7, 0.0);

  // for (size_t i = 0; i < num_ferias; ++i)
  // {
  //   std::vector<double> ferias_per_day;
  //   try
  //   {
  //     ferias_per_day = ferias_per_day_per_proc.at(sol[i]);
  //   }
  //   catch (std::out_of_range &e)
  //   {
  //     ferias_per_day = std::vector<double>(7, 0.0);
  //   }

  //   Feria *feria = items.at(i);
  //   std::vector<int> dias_funcionamiento = feria->get_dia_funcionamiento();

  //   // Para cada uno de los meses en que funciona la feria,
  //   // aumentamos el contador
  //   for (const auto &dia : dias_funcionamiento)
  //     ferias_per_day[dia]++;

  //   // Actualizamos el vector
  //   ferias_per_day_per_proc[sol[i]] = ferias_per_day;

  //   // Actualizamos el centroide
  //   for (int j = 0; j < 7; ++j)
  //     centroid[j] += ferias_per_day[j];
  // }

  // // Dividimos y normalizamos
  // for (int i = 0; i < 7; ++i)
  //   centroid[i] /= (double)ferias_per_day_per_proc.size();

  // double result = 0.0;

  // // Ahora, por cada entrada del procesador, acumulamos la
  // // diferencia con respecto al centroide
  // for (const auto &[proc, ferias_per_day] : ferias_per_day_per_proc)
  // {
  //   for (int i = 0; i < 7; ++i)
  //     result += abs(centroid[i] - ferias_per_day[i]);
  // }

  // return result;
}

// ParallelSimulation::ParallelSimulation() : Simulation(){};

ParallelSimulation::ParallelSimulation(
    const double _max_sim_time, const std::string &config_path
)
    : Simulation(_max_sim_time, config_path)
{
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
    // // this->fel->insert_event(
    //     24.0 * i, AGENT_TYPE::AMBIENTE, EVENTOS_AMBIENTE::INICIO_FERIA, 0,
    //     Message()
    // );
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
  int events_per_ss = 0;
  double window_size = 48;
  double next_window = this->fel->get_time() + window_size;
  long total_sync_time = 0;
  time_t t_init = time(NULL);

  // Ciclo principal de simulación
  while (this->gvt <= this->max_sim_time && !fel->is_empty())
  {

    std::map<std::string, int> agent_type_count = {
        {"CONSUMIDOR", 0}, {"FERIANTE", 0}, {"AGRICULTOR", 0}, {"AMBIENTE", 0}
    };

    std::map<std::string, int> event_type_count = {
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
        {"AGRICULTOR_INVENTARIO_VENCIDO", 0}
    };

    std::map<std::string, double> event_type_time = {
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
        {"AGRICULTOR_INVENTARIO_VENCIDO", 0.0}
    };

    events_per_ss = 0;
    while (this->fel->get_time() <= next_window)
    {

      current_event = fel->next_event();
      agent_type = agent_type_to_agent.at(current_event->get_type());
      ++agent_type_count[agent_type];
      ++event_type_count[event_type_to_type.at(current_event->get_process())];
      ++events_per_ss;
      // Para mostrar algo por consola
      if ((current_event->event_id % 10'000'000) == 0)
        printf(
            "[PROC %d/%d] - SIM TIME: %lf\tEVENT ID: %d\n", this->pid,
            this->nprocs, fel->get_time(), current_event->event_id
        );
      auto start = std::chrono::steady_clock::now();
      this->route_event(current_event);
      auto end = std::chrono::steady_clock::now();
      double duration_ms =
          std::chrono::duration_cast<std::chrono::microseconds>(end - start)
              .count() /
          1000.0;
      event_type_time[event_type_to_type.at(current_event->get_process())] +=
          duration_ms;
      this->fel->event_pool.release(current_event);
      // delete current_event;
    }
    // Aumentamos el contador de super pasos
    nsteps++;

    // Actualizamos los contadores globales
    for (const auto &[agent, count] : agent_type_count)
    {
      agent_type_count_global[agent] += count;
    }
    for (const auto &[event_type, count] : event_type_count)
    {
      event_type_count_global[event_type] += count;
      event_type_time_global[event_type] += event_type_time[event_type];
    }
    // Actualizamos la próxima ventana
    next_window += window_size;
    // Imprimimos la cantidad de eventos procesados en el SS
    // para efectos de debug

    printf(
        "[PROC: %d] || \t- SS: %d\t- Eventos procesados: %d\n ", this->pid,
        nsteps, events_per_ss
    );

    printf("\n[PROC: %d] || Event Timing Statistics:\n", this->pid);
    printf("%-40s %10s\n", "Event Type", "Time (ms)");
    printf("%s\n", std::string(50, '-').c_str());

    // Print each group with its events
    printf("\nFERIANTE Events:\n");
    for (const auto &pair : event_type_time)
    {
      if (pair.first.find("FERIANTE") == 0)
      {
        printf(
            "[PROC: %d] || %-40s %10.3f\n", this->pid, pair.first.c_str(),
            pair.second
        );
      }
    }

    printf("\n[PROC: %d] || CONSUMIDOR Events:\n", this->pid);
    for (const auto &pair : event_type_time)
    {
      if (pair.first.find("CONSUMIDOR") == 0)
      {
        printf(
            "[PROC: %d] || %-40s %10.3f\n", this->pid, pair.first.c_str(),
            pair.second
        );
      }
    }

    printf("\nAGRICULTOR Events:\n");
    for (const auto &pair : event_type_time)
    {
      if (pair.first.find("AGRICULTOR") == 0)
      {
        printf(
            "[PROC: %d] || %-40s %10.3f\n", this->pid, pair.first.c_str(),
            pair.second
        );
      }
    }

    // Procesamos colas de entrada y de salida

    // Mandamos mensajes de salida
    for (auto &[proc, messages] : this->out_queue)
    {
      MessageSerializer::send(messages, proc);
      messages.clear();
    }
    // auto start = std::chrono::high_resolution_clock::now();
    start = bsp_time();
    // Enviamos y recibimos todos los mensajes
    bsp_sync();
    stop = bsp_time();
    // auto stop = std::chrono::high_resolution_clock::now();
    // total_sync_time +=
    //     std::chrono::duration_cast<std::chrono::microseconds>(stop - start)
    //         .count();
    total_sync_time += (stop - start);
    std::vector<Message> incoming = MessageSerializer::receive();
    printf("INCOMING SIZE: %ld\n", incoming.size());
    for (auto msg : incoming)
      this->message_handler(msg);

    // Si se cumple la frecuencia, actualizamos
    if ((nsteps % this->GVT_CALCULATION_FREQ) == 0)
    {
      this->update_gvt();
      printf(
          "[PROC %d] || GVT: %lf - LVT: %lf - SS: %d \n", this->pid, this->gvt,
          this->fel->get_time(), nsteps
      );
    }
  }
  time_t t_end = time(NULL);

  bsp_sync();
  printf(
      "[PROC %d] WALL_TIME: %ld\tBSP_TIME: %lf\tSYNC_TIME: %ld[microseconds]\n",
      this->pid, t_end - t_init, bsp_time(), total_sync_time
  );
}

void ParallelSimulation::route_event(Event *e)
{

  // printf(
  //     "caller_id: %d caller_type: %d process: %d \n", e->get_caller_id(),
  //     e->get_type(), e->get_process()
  // );
  // Manejamos cada evento según su tipo
  switch (e->get_type())
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
          // Si el agricultor no tiene inventario válido, seguimos hasta
          // encontrar
          if (!inv.is_valid_inventory() || amount < inv.get_quantity())
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
  }
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
    ferias_per_proc = feria_partitioner.partition_items(this->nprocs);

    // Generamos un arreglo equivalente, pero solo con los IDs
    // (mejor para el envío de mensajes)

    std::unordered_map<int, std::vector<int>> ferias_id_per_proc;
    std::cout << "Ferias per proc: \n";
    for (auto const &[proc, ferias] : ferias_per_proc)
    {
      ferias_id_per_proc[proc] = std::vector<int>(ferias.size(), 0);
      std::cout << "PROC: " << proc << "\t";
      for (size_t i = 0; i < ferias.size(); ++i)
      {
        std::cout << ferias[i]->get_id() << " ";
        ferias_id_per_proc[proc].at(i) = ferias[i]->get_id();
      }
      std::cout << std::endl;
    }

    // Mandamos los mensajes

    for (int i = 0; i < this->nprocs; ++i)
      bsp_send(
          i, nullptr, ferias_id_per_proc[i].data(),
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
    prods_por_proc = product_partitioner.partition_items(this->nprocs);
    this->prods_per_proc = prods_por_proc;
    std::cout << "Prods por proc:\n";
    for (auto const &[proc, prods] : prods_por_proc)
    {
      std::cout << "PROC: " << proc << "\t";
      for (auto const prod : prods)
        std::cout << prod->get_id() << " ";
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
  printf("%s\n", output.c_str());
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

ParallelSimulation::~ParallelSimulation() { printf("Eliminando...\n"); }
