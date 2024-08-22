#ifndef EVENT_H
#define EVENT_H

#include "glob.h"
#include "message.h"
class Message;
class Agent;

enum AGENT_TYPE
{
  CONSUMIDOR = 1,
  FERIANTE = 2,
  AGRICULTOR = 3,
  AMBIENTE = 4, // Tipo de agente especial para eventos de contextos
  AGENT_COUNT = 4,
};

enum EVENTOS_FERIANTE
{
  INIT_COMPRA_MAYORISTA = 21,
  VENTA_CONSUMIDOR = 22,
  PROCESS_COMPRA_MAYORISTA = 23,
  COMPRA_MAYORISTA = 24,
  EVENTOS_FERIANTE_COUNT = 4,
};

enum EVENTOS_CONSUMIDOR
{
  BUSCAR_FERIANTE = 11, /** Evento para buscar a feriante dentro de la feria*/
  INIT_COMPRA_FERIANTE = 12, /** Representa la solicitud de compra de producto a
                           un feriante*/
  PROCESAR_COMPRA_FERIANTE = 13, /** Una vez que el feriante confirma la compra,
                               se actualiza el presupuesto/inventario*/
  COMPRA_FERIANTE = 14, /** Representa el proceso completo de compra, sin
                      considerar el paso de mensajes entre agentes*/
  EVENTOS_CONSUMIDOR_COUNT = 4,
};

enum EVENTOS_AGRICULTOR
{
  CULTIVO_TERRENO = 31,
  COSECHA = 32,
  VENTA_FERIANTE = 33,
  INVENTARIO_VENCIDO = 34,
  EVENTOS_AGRICULTOR_COUNT = 4,
};

const std::map<int, std::string> agent_type_to_agent = {
    {(int)AGENT_TYPE::CONSUMIDOR, "CONSUMIDOR"},
    {(int)AGENT_TYPE::FERIANTE, "FERIANTE"},
    {(int)AGENT_TYPE::AGRICULTOR, "AGRICULTOR"},
    {(int)AGENT_TYPE::AMBIENTE, "AMBIENTE"}
};

class Event
{
private:
  static int current_event_id;
  double time; /** Tiempo de simulación en que ocurre el evento*/
  int agent_type;
  int process;
  int caller_id; /** ID del agente, relativo a su tipo, que generó el evento*/
  Event *next = nullptr;
  Agent *caller_ptr = nullptr;
  Message msg;

public:
  int event_id;
  Event();

  Event(
      double _time = 0.0, int _agent_type = -1, int _process = -1,
      int _caller_id = -1, Message _msg = Message(),
      Agent *_caller_ptr = nullptr
  );

  /**
   * Getters
   *
   */

  double get_time() const;

  int get_type() const;

  int get_process() const;

  int get_caller_id() const;

  Event *get_next_event() const;

  Message get_message() const;

  Agent *get_caller_ptr() const;

  /**
   * Setters
   *
   */

  void set_time(double _time);

  void set_message(Message _msg);

  void set_next_event(Event *next);

  /**
   *
   * Class destructor
   * */

  ~Event();
};

#endif // !EVENT_H
