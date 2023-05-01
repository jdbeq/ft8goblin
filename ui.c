#include <stdio.h>
#include <stdlib.h>
#include "config.h"

void exit_fix_config(void) {
   printf("Please edit your config.json and try again!\n");
   exit(255);
}

int main(int argc, char **argv) {
   printf("ft8cli: A frontend to ft8md (ft8 mass decoder)\n\n");

   // This can't work without a valid configuration...
   if (!load_config())
      exit_fix_config();

   return 0;
}
