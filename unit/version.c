#include <assert.h>
#include <string.h>

#include "../src/version.h"

int main(void) {
  assert(strcmp(CLOX_VERSION, "0.1.0") == 0);
  return 0;
}
