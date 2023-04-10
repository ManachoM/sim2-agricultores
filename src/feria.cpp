#include "../includes/feria.h"

int Feria::_current_id(-1);

Feria::Feria(std::map<int, Feriante *> const &_feriantes,
             std::vector<int> const &_dias,
             Environment *_env,
             FEL *_fel)
    : feria_id(++_current_id),
      feriantes(_feriantes),
      fel(_fel),
      dia_funcionamiento(_dias),
      env(_env)
{
}

std::map<int, Feriante *> Feria::get_feriantes() const { return this->feriantes; }
