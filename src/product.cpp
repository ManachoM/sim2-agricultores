#include "../includes/product.h"

int Producto::_current_id(-1);

Producto::Producto(
    std::string const _nombre, std::vector<int> const _siembra,
    std::vector<int> const _venta, double _cosecha, short _unit,
    double _rendimiento, double _vol_feriante, double _vol_cons,
    double _prob_cons, double _precio_feria
)
    : id(++_current_id), nombre(_nombre), meses_siembra(_siembra),
      meses_venta(_venta), dias_cosecha(_cosecha), unidad(_unit),
      rendimiento(_rendimiento), volumen_feriante(_vol_feriante),
      volumen_un_consumidor(_vol_cons), prob_consumo(_prob_cons),
      precio_feria(_precio_feria) {};

std::string Producto::get_nombre() const { return this->nombre; }

short Producto::get_unidad() const { return this->unidad; }

double Producto::get_rendimiento() const { return this->rendimiento; }

double Producto::get_volumen_feriante() const { return this->volumen_feriante; }

double Producto::get_volumen_consumidor() const {
  return this->volumen_un_consumidor;
}

double Producto::get_probabilidad_consumo() const { return this->prob_consumo; }

double Producto::get_precio_feria() const { return this->precio_feria; }

std::vector<int> Producto::get_meses_siembra() const {
  return this->meses_siembra;
}

std::vector<int> Producto::get_meses_venta() const { return this->meses_venta; }

int Producto::get_id() { return this->id; }

double Producto::get_dias_cosecha() const { return this->dias_cosecha; }

std::vector<int> Producto::get_olas_calor() { return this->olas_calor; }
std::vector<int> Producto::get_sequias() { return this->sequias; }
std::vector<int> Producto::get_heladas() { return this->heladas; }
std::vector<int> Producto::get_plagas() { return this->plagas; }

void Producto::set_heladas(std::vector<int> const &_hel) {
  this->heladas = _hel;
}

void Producto::set_sequias(std::vector<int> const &_seq) {
  this->sequias = _seq;
}

void Producto::set_olas_calor(std::vector<int> const &_oc) {
  this->olas_calor = _oc;
}

void Producto::set_plagas(std::vector<int> const &_pl) { this->plagas = _pl; }

void Producto::set_precios_mes(std::vector<double> const &_pms) {
  this->precios_mes = _pms;
}

void Producto::set_costo_ha(const double &costo) { this->costo_ha = costo; }

std::vector<double> Producto::get_precios_mes() { return this->precios_mes; }

double Producto::get_costo_ha() const { return this->costo_ha; }
