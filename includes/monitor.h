/**
 * @file Monitor.h
 * @author Manuel Ignacio Manríquez (@ManachoM)
 * @brief Clase abstracta que actúa como interfaz y permite definir cualquier
 * monitor.
 * @version solo diosito sabe
 * @date 2023-02-06
 *
 * @copyright Copyright (c) 2023
 *  !! Importado directo del sim1
 */

#ifndef _MONITOR_H_
#define _MONITOR_H_

#include "glob.h"

struct SSEventRecord
{
  int ss_number;
  int proc_id;
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

  std::map<std::string, int> agent_type_count = {
      {"CONSUMIDOR", 0}, {"FERIANTE", 0}, {"AGRICULTOR", 0}, {"AMBIENTE", 0}
  };
};

struct SSTimeRecord
{
  int ss_number;
  int proc_id;
  double exec_time;
  double sync_time;
};

class Monitor
{
protected:
  std::string sim_id; /** Identificador de la simulación basado en timestamp de
                         inicio del programa*/
  std::string file_prefix; /** Prefijo con el cual se llamaran a todos los
                              archivos/carpetas asociados a una ejecución
                              específica del simulador*/
  bool debug_flag;         /** Flag para debug por consola*/
public:
  /**
   * @brief Constructor base para cualquier Monitor. Genera el sim_id y asigna
   * los atributos
   *
   * @param _file_prefix Prefijo de archivos que generará el monitor
   * @param _debug
   */
  Monitor(std::string const &file_prefix = "./out", bool _debug = false);

  /**
   * @brief Método encargado de registrar los eventos generados
   *
   * @param log Objeto json con el evento. Debe contener al menos el tipo de
   * agente, el tipo de evento y el tiempo de simulación.
   */
  virtual void write_log(json &log) = 0;

  virtual void write_duration(double t);

  virtual void write_results();

  virtual void write_params(const std::string &key, const std::string &value);

  virtual void add_event_record(SSEventRecord e);

  virtual void add_time_record(SSTimeRecord e);

  virtual ~Monitor();
};

#endif // !_MONITOR_H_
