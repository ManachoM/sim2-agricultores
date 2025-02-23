#ifndef TERRENO_H
#define TERRENO_H

#include "glob.h"

class Terreno {
private:
  static int _current_id;
  int id;
  int cod_comuna;
  double area;
  std::string comuna;
  int id_producto_plantado;

public:
  short amenaza_plaga;
  Terreno(int _cod, double _area, std::string _comuna, int _id_prod = -1);

  int get_id() const;

  int get_cod_comuna() const;

  double get_area() const;

  int get_producto() const;

  void set_producto_plantado(int _id_prod);
};

#endif // !TERRENO_H
