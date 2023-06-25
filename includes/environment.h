#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include "agent.h"
// TODO: Pasar a objeto de configuración
#define AFECTACION_HELDAS 0.3
#define AFECTACION_SEQUIAS 0.3
#define AFECTACION_OLAS_CALOR 0.1
#define AFECTACION_PLAGAS 0.3

#include "consumidor.h"
#include "event.h"
#include "fel.h"
#include "feria.h"
#include "message_queue.h"
#include "glob.h"

class Consumidor;
class Feria;
class Feriante;
class Agricultor;
class Producto;
class Monitor;

enum EVENTOS_AMBIENTE
{
  INICIO_FERIA,
  FIN_FERIA,
  CALCULO_PRECIOS,
  LIMPIEZA_MERCADO_MAYORISTA
};

class Environment
{
private:
  FEL *fel;
  std::map<int, Agricultor *> agricultores;
  std::map<int, Feria *> ferias;
  std::map<int, Consumidor *> consumidores;
  std::map<int, Feriante *> feriantes;
  std::map<int, Producto *> productos;
  std::map<int, std::vector<Consumidor *>> consumidor_dia;
  std::map<int, std::vector<Producto *>> venta_producto_mes;
  // TODO:  helada_nivel;
  std::vector<int> sequias_nivel;
  std::vector<int> oc_nivel;
  Monitor *monitor;
  MessageQueue* message_queue;


public:
  explicit Environment(FEL *_fel);

  void set_feriantes(std::map<int, Feriante *> _feriantes);

  void set_consumidores(std::map<int, Consumidor *> _cons);

  void set_ferias(std::map<int, Feria *> _ferias);

  void process_event(Event *e);

  short get_day_week();

  short get_day_month();

  short get_month();

  short get_year();

  std::map<int, Feria *> get_ferias();

  std::map<int, Feriante *> get_feriantes();

  std::map<int, Agricultor *> get_agricultores();

  std::map<int, Producto *> get_productos();

  std::map<int, std::vector<Producto *>> get_venta_producto_mes();

  int  get_nivel_heladas();

  int get_nivel_sequias();

  int  get_nivel_olas_calor();

  void  initialize_system();

  ~Environment();
};

#endif // !ENVIRONMENT_H
