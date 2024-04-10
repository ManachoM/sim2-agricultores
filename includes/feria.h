
#ifndef _FERIA_H_
#define _FERIA_H_

#include "glob.h"
#include "fel.h"
#include "environment.h"

class Feriante;
class Environment;

class Feria
{

private:
    int feria_id;
    static int _current_id;
    int num_feriantes;
    std::map<int, Feriante *> feriantes;
    FEL *fel;
    std::vector<int> dia_funcionamiento;
    std::map<int, std::unordered_set<int>> feriante_producto; /** Matriz de booleanos que almacena el índice invertido*/
    Environment *env;

public:
    Feria();

    Feria(std::vector<int> const &_dias, const int _num_feriantes, Environment *_env = nullptr, FEL *_fel = nullptr);

    std::map<int, Feriante *> get_feriantes() const;

    /**
     * @brief Devuelve lista de id's con todos los feriantes que tienen stock de un producto
     * @param _id ID del producto a indexars
     *
     * @return std::vector<int>
     */
    std::vector<int> get_feriantes_by_id(int prod_id);

    void update_index(int feriante_id, int prod_id, bool inv);

    bool is_active();

    void initialize_feria();

    std::vector<std::vector<bool>> get_index();

    int get_id() const;

    int get_num_feriantes() const;

    void set_feriantes(std::map<int, Feriante *> const &_fers);

    double get_next_active_time();
};

#endif // !_FERIA_H_
