/**
 * @file agricultor.h
 * @author Manuel Ignacio Manríquez (@ManachoM)
 * @brief
 * @version 0.1
 * @date 2023-06-29
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef AGRICULTOR_H
#define AGRICULTOR_H

#include "agent.h"
#include "environment.h"
#include "fel.h"
#include "glob.h"

class MercadoMayorista;

class Agricultor : public Agent
{
private:
    FEL *fel;
    std::vector<Inventario> inventario;
    Terreno *terreno;
    MercadoMayorista *mercado;
    bool tiene_seguro;

public:
    Agricultor(FEL *_fel = nullptr, Terreno *_terr = nullptr, bool _seg = false);
    void process_event(Event *e) override;
};


// Templates para selección de elemento de forma aleatoria en contenedor STL

// * TODO: Revisar https://gist.github.com/cbsmith/5538174 para una mejor implementación
template<typename Iter, typename RandomGenerator>
Iter select_randomly(Iter start, Iter end, RandomGenerator& g) {
    std::uniform_int_distribution<> dis(0, std::distance(start, end) - 1);
    std::advance(start, dis(g));
    return start;
}

template<typename Iter>
Iter select_randomly(Iter start, Iter end) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    return select_randomly(start, end, gen);
}

#endif // !AGRICULTOR_H