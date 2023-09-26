#ifndef PRODUCT_H
#define PRODUCT_H

#include "glob.h"

enum UNIT_TYPES
{
    KILOS,
    UNIDAD
};

class Producto
{
private:
    static int _current_id;
    int id;
    std::string const nombre;
    std::vector<int> const meses_siembra;
    std::vector<int> const meses_venta;
    double dias_cosecha;
    short unidad;
    double rendimiento;
    double volumen_feriante;
    double volumen_un_consumidor;
    double prob_consumo;
    std::vector<int> heladas;
    std::vector<int> sequias;
    std::vector<int> olas_calor;
    std::vector<int> plagas;

public:
    Producto(std::string _nombre,
             std::vector<int> _siembra,
             std::vector<int> _venta,
             double _cosecha,
             short _unit,
             double _rendimiento,
             double _vol_feriante,
             double _vol_cons,
             double _prob_cons);

    std::string get_nombre() const;

    short get_unidad() const;

    double get_rendimiento() const;

    double get_volumen_feriante() const;

    double get_volumen_consumidor() const;

    double get_probabilidad_consumo() const;

    std::vector<int> get_meses_siembra() const;

    std::vector<int> get_meses_venta() const;

    int get_id() const;

    double get_dias_cosecha() const;

    void set_heladas(std::vector<int> const &_hel);

    void set_sequias(std::vector<int> const &_seq);

    void set_olas_calor(std::vector<int> const &_oc);

    void set_plagas(std::vector<int> const &_pl);

    std::vector<int> get_heladas();

    std::vector<int> get_sequias();

    std::vector<int> get_olas_calor();

    std::vector<int> get_plagas();
};

#endif // !PRODUCT_H