
#ifndef FERIA_H
#define FERIA_H

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
        std::map<int, Feriante *> feriantes;
        FEL *fel;
        std::vector<int> dia_funcionamiento;
        std::vector< std::vector<bool> > feriante_producto; /** Matriz de booleanos que almacena el índice invertido*/
        Environment *env;

    public:
        Feria();

        Feria(std::vector<int> const & _dias, Environment *_env = nullptr, FEL *_fel = nullptr);

        std::map<int, Feriante*> get_feriantes() const;

        /**
         * @brief Devuelve lista de id's con todos los feriantes que tienen stock de un producto
         * @param _id ID del producto a indexars
         * 
         * @return std::vector<int> 
         */
        std::vector<int> get_feriantes_by_id(int _id);

        void update_index(int _id = -1);

        bool is_active();

        void initialize_feria();

        std::vector< std::vector<bool> > get_index();

        int get_id() const ;

        void set_feriantes(std::map<int, Feriante*> const & _fers);
};


#endif // !FERIA_H

