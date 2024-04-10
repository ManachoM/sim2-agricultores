/**
 *   @author: Manuel Ignacio Manríquez<br>
 *   Descripción: Clase para representar a un (1) feriante
 *   como agennte de la simulación. Los feriantes son aquellos
 *   que se dedican a comprar a Agricultores en Mercacdos Mayoristas
 *
 */
#ifndef FERIANTE_H
#define FERIANTE_H

#include "agent.h"
#include "environment.h"
#include "event.h"
#include "fel.h"
// TODO #include "MercadoMayorista.h"
#include "inventory.h"
#include "feria.h"
#include "product.h"
#include "glob.h"

class Agricultor;
class Feria;
class MercadoMayorista;
class Inventario;

class Feriante : public Agent
{
private:
    static int current_feriante_id;

    int feriante_id;

    FEL *fel; /** Referencia  a la lista de eventos global */

    std::map<int, Inventario> inventario; /** Mapa del tipo <id_producto, Inventario> con todo el inventario del feriante */

    int feria_id; 

    std::vector<int> productos; /** Arreglo con los productos que el feriante venderá*/


    virtual std::vector<int> choose_product() = 0;

    virtual double purchase_amount(const int prod_id) = 0;

    virtual Agricultor *choose_agricultor(const int prod_id, const double amount) = 0;

    virtual void finish_purchase() = 0;

    void process_init_compra();

    void process_resp_agricultor(const Event *e, json &log);

    void process_venta_feriante(const Event *e);

public:
    Feriante(FEL *list = nullptr, MercadoMayorista *_mer = nullptr, int feria_id = -1);
    MercadoMayorista *mercado; /** Puntero al mercado mayorista del sistema*/

    /**
     *   Método 100% virtual que se debe implementar desde
     *   la clase Agente. Se encarga de modificar el estado
     *   interno de la instancia.
     *   @param e Evento a ser procesado
     */
    void process_event(Event *e) override;

    /*!
        Getter para obtener el mapa <id_producto, Inventario> del feriante.
        @returns map<id_producto, Inventario> con los inventarios del feritante
    */
    std::map<int, Inventario> get_inventario() const;

    /*!
        Método para consultar por el inventario de un determinado producto.
        @param idProducto Id del producto por el cual se consulta.
        @return Inventario de ese producto que mantiene el feriante. Si el feriante no tiene inventario de ese producto, retorna una instancia de inventario con todo 0 excepto el id_producto.
    */
    Inventario get_inventario_by_id(int idProducto);

    void set_feria(const int feria_id);

    /**
     * @brief Getter de la primera feria a la que pertenece el feriante
     *
     * @return Feria*
     */
    Feria *get_feria();

    int get_feriante_id();

    ~Feriante();
};

#endif //! FERIANTE_H