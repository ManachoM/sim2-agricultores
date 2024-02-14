/**
 * @file mercado_mayorista.h
 * @author ManachoM (manuel.manriquez.b@usach.cl)
 * @brief
 * @version 0.1
 * @date 2023-09-24
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef _MERCADO_MAYORISTA_
#define _MERCADO_MAYORISTA_

#include "glob.h"
#include "agricultor.h"
#include "boolean_matrix.h"
#include "environment.h"

class Agricultor;

class MercadoMayorista
{
private:
    Environment *env;
    BooleanMatrix prod_mat;

public:
    MercadoMayorista(Environment *_env);

    void update_index(int agr_id, int prod_id, bool inv);

    void reset_index();

    std::vector<Agricultor> get_agricultor_por_prod(int prod_id);
};

#endif // !_MERCADO_MAYORISTA_