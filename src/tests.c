#include "alisp/vm.h"

int main() {
  struct a_vm vm;
  a_vm_init(&vm);
  a_vm_deref(&vm);
  return 0;
}
