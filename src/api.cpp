#include "api.h"

DB_functions_t *deadbeef;

DB_functions_t *get_API_pointer() {
  return deadbeef;
}

void set_API_pointer(DB_functions_t *api) {
  deadbeef = api;
}
