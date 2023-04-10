#ifndef CONSUMIDOR_H
#define CONSUMIDOR_H

#include "agent.h"
#include "fel.h"
#include "glob.h"

enum EVENTOS_CONSUMIDOR
{
  SOLICITAR_COMPRA_FERIANTE,
  RESPUESTA_COMPRA_FERIANTE
};

class Consumidor : public Agent
{
private:
  static int current_consumer_id;
  int consumer_id;
  FEL *fel;     // Ptr a FEL con eventos del simulador
  int id_feria; // ID de la feria a la que asiste el Consumidor
  int cant_integrantes = 1;
  int choose_product();

public:
  Consumidor(FEL *_fel = nullptr, int _feria = -1, int _cant_integrantes = 1);

  void process_event(Event *e) override;

  void initialize_purchase();

  void set_feria(int _feria_id);

  int get_feria();

  int get_consumer_id();
};

#endif // !CONSUMIDOR_H
