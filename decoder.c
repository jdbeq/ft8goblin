#include <stdio.h>
#include <stdlib.h>
#include "config.h"

void exit_fix_config(void) {
   printf("Please edit your config.json and try again!\n");
   exit(255);
}

int main(int argc, char **argv) {
   printf("ft8decoder: Sub-process used to decode ft8 messages from audio clips from various sources\n\n");

   if (argc != 2) {
      fprintf(stderr, "ERROR: Only one argument is expected, the name of the band to decode on!\n");
      fprintf(stderr, "All other settings are loaded from the configuration file config.json in the current directory or /etc/ft8goblin/\n");
      exit(1);
   }

   // This can't work without a valid configuration...
   if (!load_config())
      exit_fix_config();

   return 0;
}
