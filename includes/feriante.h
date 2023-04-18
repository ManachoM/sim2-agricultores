/**
 *   @author: Manuel Ignacio Manríquez<br>
 *   Descripción: Clase para representar a un (1) feriante
 *   como agennte de la simulación. Los feriantes son aquellos
 *   que se dedican a comprar a Agricultores en Mercacdos Mayoristas
 *
 *  !!Importado directamente desde sim1
 */
#ifndef FERIANTE_H
#define FERIANTE_H

#include "agent.h"
#include "environment.h"
#include "event.h"
#include "fel.h"
// TODO #include "MercadoMayorista.h"
// TODO #include "Inventario.h"
#include "feria.h"
#include "product.h"
#include "glob.h"

class Feria;
class MercadoMayorista;
class Inventario;

enum EVENTOS_FERIANTE
{
    COMPRA_MAYORISTA,
    VENTA_CONSUMIDOR,
    LIMPIEZA_INVENTARIO
};

class Feriante : public Agent
{
private:
    FEL *event_list; /** Referencia  a la lista de eventos global */

    std::map<int, Inventario> inventario; /** Mapa del tipo <id_producto, Inventario> con todo el inventario del feriante */

    std::map<int, Feria *> ferias; /** Mapa con todas las ferias a las que atiende el feriante, del tipo <id_feria, Feria*> */

    double ultima_limpieza = 0.0; /** Último tiempo de simulación en que se limpió el inventario del feriante. Representa la durabilidad del inventario.*/

    Environment *env; /** Puntero al sistema */

    std::vector<int> productos; /** Arreglo con los productos que el feriante venderá*/

    MercadoMayorista *mercado; /** Puntero al mercado mayorista del sistema*/

public:
    Feriante(FEL *list = nullptr, Environment *_env = nullptr, MercadoMayorista *_mer = nullptr);

    /**
     *   Método 100% virtual que se debe implementar desde
     *   la clase Agente. Se encarga de modificar el estado
     *   interno de la instancia.
     *   @param e Evento a ser procesado
     */
    void process_event(Event *e) override;

    /*!
        Método que representa el acto de comprarle al feriante.
        En la práctica, esto implica reducir su inventario y aumentar su
        dinero. Aún queda decidir cómo determinar la cantidad de producto a comprar
        @param  idProducto ID del producto a comprar.
        @param cantidad Cantidad de producto - en su respectiva unidad - a comprar por el consumidor
        @return Booleano que es True cuando se concretó la compra, False si no tenía stock

    */
    bool comprar_by_id(int idProducto, double cantidad = -1.0);

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

    /*!
        Setter para agregar feria al mapa de ferias del feriante.
        @param _feria Puntero a la feria que debe ser agregada al mapa
    */
    void add_feria(Feria *_feria);

    /**
     * @brief Método llamado para generar los eventos de compra de los agricultores.
     * Esta toma de decisiones debería estar influenciada por el calendario y otras variables, como precio
     * del mercado mayorista. Esta implementación es bastante ingenua en ese sentido.
     *
     */
    void comprar_productos();

    /**
     * @brief Getter de la primera feria a la que pertenece el feriante
     *
     * @return Feria*
     */
    Feria *get_feria();

    ~Feriante();
};

#endif //! FERIANTE_H