#include "../includes/heap_fel.h"

HeapFEL::HeapFEL()
{
  // // Inicializamos los mapas para conteo -- TODO: Revisar con menos tuto
  // for (int agent = AGENT_TYPE::CONSUMIDOR; agent < AGENT_TYPE::AGENT_COUNT;
  //      ++agent)
  // {
  //   switch (agent)
  //   {
  //   case AGENT_TYPE::CONSUMIDOR:
  //   {
  //     for (int event_type = EVENTOS_CONSUMIDOR::BUSCAR_FERIANTE;
  //          event_type < EVENTOS_CONSUMIDOR::EVENTOS_CONSUMIDOR_COUNT;
  //          ++event_type)
  //     {
  //       this->agent_event_type_count[agent][event_type] = 0;
  //     }
  //     break;
  //   }
  //   case AGENT_TYPE::FERIANTE:
  //   {
  //     for (int event_type = EVENTOS_FERIANTE::INIT_COMPRA_MAYORISTA;
  //          event_type < EVENTOS_FERIANTE::EVENTOS_FERIANTE_COUNT;
  //          ++event_type)
  //     {

  //       this->agent_event_type_count[agent][event_type] = 0;
  //     }
  //     break;
  //   }
  //   case AGENT_TYPE::AGRICULTOR:
  //   {
  //     for (int event_type = EVENTOS_AGRICULTOR::CULTIVO_TERRENO;
  //          event_type < EVENTOS_AGRICULTOR::EVENTOS_AGRICULTOR_COUNT;
  //          ++event_type)
  //     {

  //       this->agent_event_type_count[agent][event_type] = 0;
  //     }
  //     break;
  //   }
  //   default:
  //     break;
  //   }
  // }
}

void HeapFEL::insert_event(
    double _time, int _agent_type, int _proc, int _caller_id, Message _msg,
    Agent *_caller_ptr
)
{
  Event *e = new Event(
      _time + this->clock, _agent_type, _proc, _caller_id, _msg, _caller_ptr
  );
  this->pq.push(e);
  // this->agent_event_type_count[_agent_type][_proc]++;
  if (_agent_type == AGENT_TYPE::FERIANTE &&
      _proc == EVENTOS_FERIANTE::PROCESS_COMPRA_MAYORISTA)
    this->num_process_resp_agr++;

  if (_agent_type == AGENT_TYPE::AGRICULTOR &&
      _proc == EVENTOS_AGRICULTOR::VENTA_FERIANTE)
    this->num_venta_feriante++;
}

void HeapFEL::insert_event(Event *e)
{
  this->pq.push(e);

  if (e->get_type() == AGENT_TYPE::FERIANTE &&
      e->get_process() == EVENTOS_FERIANTE::PROCESS_COMPRA_MAYORISTA)
    this->num_process_resp_agr++;

  if (e->get_type() == AGENT_TYPE::AGRICULTOR &&
      e->get_process() == EVENTOS_AGRICULTOR::VENTA_FERIANTE)
    this->num_venta_feriante++;
}

Event *HeapFEL::next_event()
{
  Event *e = this->pq.top();

  // TODO: Add exceptions in case of time discrepancies
  //

  this->clock = e->get_time();
  this->pq.pop();
  // this->agent_event_type_count[e->get_type()][e->get_process()]--;
  if (e->get_type() == AGENT_TYPE::FERIANTE &&
      e->get_process() == EVENTOS_FERIANTE::PROCESS_COMPRA_MAYORISTA)
    this->num_process_resp_agr--;
  else if (e->get_type() == AGENT_TYPE::AGRICULTOR &&
           e->get_process() == EVENTOS_AGRICULTOR::VENTA_FERIANTE)
    this->num_venta_feriante--;
  return e;
}

double HeapFEL::get_time() { return this->clock; }

bool HeapFEL::is_empty() { return this->pq.empty(); }

HeapFEL::~HeapFEL(){};
