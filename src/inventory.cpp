#include "../includes/inventory.h"

int Inventario::_current_id(0);

Inventario::Inventario(double _creation_time,
                       double _exp_time, int _prod_id, double _q) : creation_time(_creation_time), expire_time(_exp_time),
                                                                    product_id(_prod_id), quantity(_q){};

int Inventario::get_product_id() const
{
    return this->product_id;
}

double Inventario::get_creation_time() const
{
    return this->creation_time;
}

double Inventario::get_expire_time() const { return this->expire_time; }

double Inventario::get_quantity() const { return this->quantity; }

bool Inventario::is_valid_inventory() const
{
    return (this->creation_time == 0.0 && this->expire_time == 0.0) || this->product_id == -1 || this->quantity == 0.0;
}

void Inventario::set_quantity(double _q) { this->quantity = _q; }
