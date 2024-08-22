#include "../includes/terreno.h"

int Terreno::_current_id(-1);

Terreno::Terreno(int _cod, double _area, std::string _comuna, int _id_prod)
    : id(++_current_id), cod_comuna(_cod), area(_area), comuna(_comuna),
      id_producto_plantado(_id_prod)
{
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> d(0, 3);
  this->amenaza_plaga = d(gen);
}

int Terreno::get_id() const { return this->id; }

int Terreno::get_cod_comuna() const { return this->cod_comuna; }

double Terreno::get_area() const { return this->area; }

int Terreno::get_producto() const { return this->id_producto_plantado; }

void Terreno::set_producto_plantado(int _id)
{
  this->id_producto_plantado = _id;
}
