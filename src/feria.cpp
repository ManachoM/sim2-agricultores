#include "../includes/feria.h"

int Feria::_current_id(-1);

Feria::Feria() : feria_id(++_current_id) {}

Feria::Feria(
             std::vector<int> const &_dias,
             Environment *_env,
             FEL *_fel)
    : feria_id(++_current_id),
      fel(_fel),
      dia_funcionamiento(_dias),
      env(_env)
{
}

std::map<int, Feriante *> Feria::get_feriantes() const { return this->feriantes; }

int Feria::get_id() const { return this->feria_id; }


void Feria::set_feriantes(std::map<int, Feriante*> const & _fers)
{
  this->feriantes = _fers;
}