#include "config.h"
#include "util.h"
// toggle an intbool (<= 0 is off, >= 1 is on)
void toggle(int *v) {
   if (*v <= 0)
      *v = 1;
   else if (*v >= 1)
      *v = 0;
}
