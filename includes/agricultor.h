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
#include "inventory.h"
#include "product.h"
#include "terreno.h"

class MercadoMayorista;
class Terreno;

class Agricultor : public Agent
{
private:
    static int curr_agricultor_id;
    int agricultor_id;
    FEL *fel;
    std::map<int, Inventario> inventario;
    Terreno *terreno;
    MercadoMayorista *mercado;
    virtual int const choose_product() = 0;
    void process_cultivo_event(const Event *e, json log);
    void process_cosecha_event(const Event *e, json log);
    void process_venta_feriante_event(const Event *e);
    void process_inventario_vencido_event(const Event *e, json log);

public:
    Agricultor(FEL *_fel = nullptr, Terreno *_terr = nullptr);
    void process_event(Event *e) override;
    std::map<int, Inventario> get_inventory() const;
    int get_agricultor_id() const;
};

// Templates para selección de elemento de forma aleatoria en contenedor STL

// * TODO: Revisar https://gist.github.com/cbsmith/5538174 para una mejor implementación
template <typename Iter, typename RandomGenerator>
Iter select_randomly(Iter start, Iter end, RandomGenerator &g)
{
    std::uniform_int_distribution<> dis(0, std::distance(start, end) - 1);
    std::advance(start, dis(g));
    return start;
}

template <typename Iter>
Iter select_randomly(Iter start, Iter end)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    return select_randomly(start, end, gen);
}

#endif // !AGRICULTOR_H