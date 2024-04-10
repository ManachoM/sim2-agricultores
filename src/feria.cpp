#include "../includes/feria.h"

int Feria::_current_id(-1);

Feria::Feria() : feria_id(++_current_id) {}

Feria::Feria(
    std::vector<int> const &_dias,
    const int _num_feriantes,
    Environment *_env,
    FEL *_fel)
    : feria_id(++_current_id),
      num_feriantes(_num_feriantes),
      fel(_fel),
      dia_funcionamiento(_dias),
      env(_env)
{
  auto cant_prods = this->env->get_productos().size();
  for (int i = 0; i < cant_prods; ++i)
  {
    this->feriante_producto.insert({i, std::unordered_set<int>{}});
  }
}

std::map<int, Feriante *> Feria::get_feriantes() const { return this->feriantes; }

std::vector<int> Feria::get_feriantes_by_id(int prod_id)
{
  std::vector<int> return_vector;
  std::unordered_set<int> fers = this->feriante_producto.at(prod_id);
  return_vector.reserve(fers.size());
  for(auto it = fers.begin(); it != fers.end(); )
  {
    return_vector.push_back(std::move(fers.extract(it++).value()));
  } 
  return return_vector;

}

int Feria::get_id() const { return this->feria_id; }

void Feria::set_feriantes(std::map<int, Feriante *> const &_fers)
{
  this->feriantes = _fers;
}

int Feria::get_num_feriantes() const { return this->num_feriantes; }

bool Feria::is_active()
{
  for (int i : this->dia_funcionamiento)
  {
    if ((int)this->env->get_day_week() == i)
      return true;
  }
  return false;
}

void Feria::initialize_feria()
{
  for (auto const &[fer_id, fer_ptr] : this->feriantes)
    this->fel->insert_event(
        1.0, AGENT_TYPE::FERIANTE, EVENTOS_FERIANTE::INIT_COMPRA_MAYORISTA, fer_id, Message(), fer_ptr);
}

double Feria::get_next_active_time()
{
  // printf("oli %p\n", this->env);
  int dia_actual = (int)(this->env->get_day_week());
  int nextDay;
  for (int i : this->dia_funcionamiento)
  {
    if (i - dia_actual > 0 && i - dia_actual < nextDay)
      nextDay = i - dia_actual;
  }

  return nextDay;
}

void Feria::update_index(int feriante_id, int prod_id, bool inv)
{
  std::unordered_set<int> prod_feriantes = this->feriante_producto.at(prod_id);
  if (inv)
    prod_feriantes.insert(feriante_id);
  else
    prod_feriantes.erase(feriante_id);

  this->feriante_producto.at(prod_id) = prod_feriantes;
}
