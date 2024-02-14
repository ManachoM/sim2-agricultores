/**
 * @file AggregatedMonitor.h
 * @author Manuel Ignacio Manríquez (@ManachoM)
 * @brief Monitor que procesa eventos agregando múltiple métricas durante la simulación. Finalmente, entrega todo en un solo JSON.
 * @version kien save
 * @date 2023-02-06
 * 
 * @copyright Copyright (c) 2023
 * 
 * !! Importado directamente del sim1 u_u
 */

#ifndef _AGGREGATION_MONITOR_H_
#define _AGGREGATION_MONITOR_H_

#include "glob.h"
#include "monitor.h"

class AggregationMonitor : public Monitor
{
    private:
        json aggregated_logs; /** Objeto que almacenará los valores agregados por mes de cada tipo de agente*/
        short last_recorded_month = 0; /** Último mes cuya entrada fue registrada en el objeto. Si el valor entrante es distinto a este, debe generarse una entrada para un nuevo mes*/
        std::map<std::string, std::map<int, std::map<std::string, double>>> agg_logs; /** Objeto que almacenará los valores por mes*/
    public:

        /**
         * @brief Constructor de la clase. Pasa los parámetros a la superclase.
         * 
         * @param _file_prefix Prefijo que identifica a esta instancia de simulación
         * @param _debug Flag para indicar si se deben imprimir los eventos procesados por consola
         */
        AggregationMonitor(std::string const &_file_prefix, bool _debug);

        /**
         * @brief Método que recibe los eventos y los procesa
         * 
         * @param log Objeto json con el evento. Debe contener al menos el tipo de agente, el tipo de evento y el tiempo de simulación.
         */
        void write_log(json log) override;

        /** 
         * Destructor de la clase.
        */
        ~AggregationMonitor() final;

};

#endif // !_AGGREGATION_MONITOR_H_