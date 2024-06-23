#ifndef _CONSUMIDOR_H_
#define _CONSUMIDOR_H_

#include "agent.h"
#include "fel.h"
#include "feriante.h"
#include "glob.h"

class Feriante;

class Consumidor : public Agent {
private:
  static int current_consumer_id;
  int consumer_id = ++current_consumer_id;
  FEL *fel;     // Ptr a FEL con eventos del simulador
  int id_feria; // ID de la feria a la que asiste el Consumidor
  virtual std::vector<int> choose_product() = 0;
  virtual double purchase_amount(const int prod_id) = 0;
  virtual Feriante *choose_feriante(const int prod_id, const double amount) = 0;
  virtual void finish_purchase() = 0;
  void process_init_compra();
  void process_resp_feriante(const Event *e, json &log);
  void process_compra_feriante(const Event *e, json &log);

public:
  Consumidor(FEL *_fel = nullptr, int _feria = -1);

  void process_event(Event *e) override;

  void set_feria(int _feria_id);

  int get_feria() const;

  int get_consumer_id() const;
};

#endif // !_CONSUMIDOR_H_
