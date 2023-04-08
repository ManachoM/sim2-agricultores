/**
 *
 *
 *
 *
 * */

#include "../includes/main.h"

int main(int argc, char *argv[]) {
  printf("%s\n", "hello world");
  auto cons = Consumidor();
  cons.process_event(nullptr);
  printf("Identificador del agente %d, identificador del consumidor %d\n",
         cons.get_id(), cons.get_consumer_id());

  SimConfig * s1 = SimConfig::get_instance("ola");
  SimConfig * s2 = SimConfig::get_instance("oli mundo");

  printf("s1: %s \ts2: %s\n", s1->get_config_file_path().c_str(), s2->get_config_file_path().c_str());
  return EXIT_SUCCESS;
}
