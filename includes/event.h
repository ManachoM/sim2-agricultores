#ifndef EVENT_H
#define EVENT_H

#include "glob.h"
#include "message.h"
class Message;
class Agent;

enum AGENT_TYPE
{
  CONSUMIDOR,
  FERIANTE,
  AGRICULTOR,
  AMBIENTE // Tipo de agente especial para eventos de contextos
};

enum EVENTOS_FERIANTE
{
    COMPRA_MAYORISTA,
    VENTA_CONSUMIDOR,
    LIMPIEZA_INVENTARIO
};


enum EVENTOS_CONSUMIDOR
{
  BUSCAR_FERIANTE,      /** Evento para buscar a feriante dentro de la feria*/
  INIT_COMPRA_FERIANTE, /** Representa la solicitud de compra de producto a un feriante*/
  FIN_COMPRA_FERIANTE   /** Una vez que el feriante confirma la compra, se actualiza el presupuesto/inventario*/
};

class Event
{
private:
  double time; /** Tiempo de simulación en que ocurre el evento*/
  int agent_type;
  int process;
  int caller_id; /** ID del agente, relativo a su tipo, que generó el evento*/
  Event *next = nullptr;
  Agent *caller_ptr = nullptr;
  Message msg;

public:
  Event();

  Event(double _time = 0.0, int _agent_type = -1, int _process = -1,
        int _caller_id = -1, Message _msg = Message(),
        Agent *_caller_ptr = nullptr);

  /**
   * Getters
   *
   */

  double get_time();

  int get_type();

  int get_process();

  int get_caller_id();

  Event *get_next_event();

  Message get_message();

  Agent *get_caller_ptr();

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
