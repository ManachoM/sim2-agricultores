/**
 * @file single_consumer.h
 * @author Manuel Ignacio Manríquez (@ManachoM)
 * @brief
 * @version 0.1
 * @date 2023-06-25
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef SINGLE_CONSUMER_H
#define SINGLE_CONSUMER_H

#include "agent.h"
#include "glob.h"
#include "fel.h"
#include "feriante.h"
#include "product.h"
#include "environment.h"
#include "feria.h"
#include "monitor.h"

class Agent;

/**
 * @brief Clase que representa un consumidor único - consume por una sola persona
 *        Sigue el patrón de consumo determinado por la probabilidad de consumo para
 *        cada producto
 *
 */
class SingleConsumer : public Agent
{

private:
    static int current_cons_id;
    int consumer_id;
    FEL *fel;                               /** Puntero a la lista de eventos*/
    int id_feria;                           /** Identificador de la feria a la que asiste el consumidor*/
    std::vector<std::vector<int>> feriantes_consultados; /** "Memoria" del consumidor, con todos los feriantes a los que se le intentó preguntar por un producto*/
    int choose_product();
    Feria *feria;

public:
    SingleConsumer(FEL *_fel = nullptr, int _fer = -1);

    void process_event(Event *e) override;

    void initialize_purchase();

    void set_feria(int _feria_id);

    int get_feria() const;

    int get_consumer_id() const;
};

#endif // !SINGLE_CONSUMER_H