/*
 * here we provide for using hamlib to control an attached transceiver
 *
 * this is usually used in conjunction with alsa/pulse/sound_io...
 *
 * ft8goblin and sigcapd both use this, to allow tunable source/sinks via hamlib
 */
#include "config.h"
#include <stdio.h>
#include <string.h>
#include <hamlib/amplifier.h>
#include <hamlib/rig.h>
#include <hamlib/rotator.h>

//
// We have to be careful to ensure all data is wrapped in a Hamlib object...
// This is the only way we can support multiple radios.
//
extern int rig_raw2val();

typedef struct Hamlib {
   RIG 		*rig;
   char		*path;
   int		model;
   freq_t	freq;
   value_t	raw_strength, raw_power, strength;
   char		*info_buf;
   float	s_meter;
   int		status, retcode;
   unsigned int	mwpower;
   rmode_t	mode;
   pbwidth_t	width;
} Hamlib;

Hamlib *hamlib_init(int model, const char *path) {
   Hamlib *r = NULL;

   if ((r = malloc(sizeof(Hamlib))) == NULL) {
      fprintf(stderr, "hamlib_init: out of memory!\n");
      exit(ENOMEM);
   }
   memset(r, 0, sizeof(Hamlib));

   // XXX: we need to store this in cfg:devices/$name/hamlib-debuglevel
   rig_set_debug(RIG_DEBUG_TRACE);

   // if model isn't set, try rigctld via localhost
   const char *mypath = NULL;
   if (model == 0) {
      model = RIG_MODEL_NETRIGCTL;
      mypath = "127.0.0.1:4532";
   } else {
      mypath = path;
   }

   // fill the structure
   r->rig = rig_init(model);
   r->model = model;

   // only duplicate the path, if provided (don't call strdup(NULL)...
   if (mypath != NULL) {
      r->path = strdup(mypath);
   }

   return r;
}
