/**
 *
 *
 *
 *
 * */

#include "../includes/main.h"
#include <cstdlib>

int main(int argc, char *argv[]) {
  printf("%s\n", "hello world");
  Consumidor cons = Consumidor();
  cons.process_event(nullptr);
  printf("Identificador del agente %d, identificador del consumidor %d\n",
         cons.get_id(), cons.get_consumer_id());

  return EXIT_SUCCESS;
}
