/**
 * @file inventory.h
 * @author @ManachoM
 * @brief
 * @version 0.1
 * @date 2023-05-02
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef INVENTORY_H
#define INVENTORY_H

class Inventario
{
private:
    int id;
    static int _current_id;
    double creation_time;
    double expire_time;
    int product_id;
    double quantity;

public:
    Inventario(double _creation_time = 0.0,
               double _expire_time = 0.0,
               int product_id = -1,
               double _quantity = 0.0);


    int get_product_id() const;

    double get_creation_time() const;

    double get_expire_time() const;

    double get_quantity() const;

    bool is_valid_inventory() const;

    void set_quantity(double _q);
};

#endif // !INVENTORY_H