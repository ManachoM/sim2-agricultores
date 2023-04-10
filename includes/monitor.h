/**
 * @file Monitor.h
 * @author Manuel Ignacio Manríquez (@ManachoM)
 * @brief Clase abstracta que actúa como interfaz y permite definir cualquier monitor. 
 * @version solo diosito sabe
 * @date 2023-02-06
 *
 * @copyright Copyright (c) 2023
 *  !! Importado directo del sim1
 */

#ifndef _MONITOR_H_
#define _MONITOR_H_

#include "glob.h"

class Monitor
{
protected:
    std::string sim_id;      /** Identificador de la simulación basado en timestamp de inicio del programa*/
    std::string file_prefix; /** Prefijo con el cual se llamaran a todos los archivos/carpetas asociados a una ejecución específica del simulador*/
    bool debug_flag;    /** Flag para debug por consola*/
public:
    /**
     * @brief Constructor base para cualquier Monitor. Genera el sim_id y asigna los atributos
     *
     * @param _file_prefix Prefijo de archivos que generará el monitor
     * @param _debug
     */
    Monitor(std::string const &file_prefix = "./out", bool _debug = false);

    /**
     * @brief Método encargado de registrar los eventos generados
     *
     * @param log Objeto json con el evento. Debe contener al menos el tipo de agente, el tipo de evento y el tiempo de simulación.
     */
    virtual void writeLog(json log) = 0;

    virtual ~Monitor();
};

#endif // !_MONITOR_H_