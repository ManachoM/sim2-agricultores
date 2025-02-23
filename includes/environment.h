#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include "agent.h"
// TODO: Pasar a objeto de configuración
#define AFECTACION_HELDAS     0.3
#define AFECTACION_SEQUIAS    0.3
#define AFECTACION_OLAS_CALOR 0.1
#define AFECTACION_PLAGAS     0.3

#include "consumidor.h"
#include "consumidor_factory.h"
#include "event.h"
#include "fel.h"
#include "feriante.h"
#include "message_queue.h"
#include "product.h"
#include "terreno.h"

class Agricultor;
class Consumidor;
class ConsumidorFactory;
class Feria;
class Feriante;
class Monitor;
class Producto;
class Terreno;

enum EVENTOS_AMBIENTE {
  INICIO_FERIA,
  FIN_FERIA,
  CALCULO_PRECIOS,
  LIMPIEZA_MERCADO_MAYORISTA
};

class Environment {
private:
  FEL *fel;
  std::vector<Consumidor *> consumidores_arr;
  std::vector<Feriante *> feriante_arr;
  std::vector<Agricultor *> agricultor_arr;
  std::vector<Feria *> feria_arr;
  std::unordered_map<int, Agricultor *> agricultores;
  std::unordered_map<int, Agricultor *> agricultor_por_id_relativo;
  std::unordered_map<int, Feria *> ferias;
  std::unordered_map<int, Consumidor *> consumidores;
  std::unordered_map<int, Feriante *> feriantes;
  std::unordered_map<int, Producto *> productos;
  std::unordered_map<int, Terreno *> terrenos;
  std::unordered_map<int, std::vector<Consumidor *>> consumidor_dia;
  std::unordered_map<int, std::vector<Producto *>> venta_producto_mes;
  std::unordered_map<int, std::vector<Producto *>> siembra_producto_mes;
  json helada_nivel;
  std::vector<int> sequias_nivel;
  std::vector<int> oc_nivel;
  Monitor *monitor;
  MessageQueue *message_queue;
  void read_products();
  void read_ferias();
  void read_terrenos();
  void initialize_agents(MercadoMayorista *mercado);

public:
  Environment(FEL *_fel, Monitor *_monitor);

  Consumidor *get_consumidor(int consumidor_id);

  Feriante *get_feriante(int feriante_id);

  Agricultor *get_agricultor(int agro_id);

  Feria *get_feria(int feria_id);

  void set_feriantes(std::unordered_map<int, Feriante *> _feriantes);

  void set_consumidores(std::unordered_map<int, Consumidor *> _cons);

  void set_ferias(std::unordered_map<int, Feria *> _ferias);

  void set_productos(std::unordered_map<int, Producto *> _prods);

  void set_agricultores(std::unordered_map<int, Agricultor *> agros);

  void set_heladas_nivel(json hn);

  void set_sequias_nivel(std::vector<int> sn);

  void set_oc_nivel(std::vector<int> ocn);

  void process_event(Event *e);

  short get_day_week();

  short get_day_month();

  short get_month();

  short get_year();

  std::unordered_map<int, Feria *> get_ferias();

  std::unordered_map<int, Feriante *> get_feriantes() const;

  std::unordered_map<int, Agricultor *> get_agricultores();

  std::unordered_map<int, Agricultor *> get_agricultores_rel();

  std::unordered_map<int, Consumidor *> get_consumidores();

  std::unordered_map<int, Producto *> get_productos();

  std::unordered_map<int, std::vector<Producto *>> get_venta_producto_mes();

  std::unordered_map<int, std::vector<Producto *>> get_siembra_producto_mes();

  int get_nivel_heladas();

  int get_nivel_sequias();

  int get_nivel_olas_calor();

  void initialize_system();

  ~Environment();
};

#endif // !ENVIRONMENT_H
