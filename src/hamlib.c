/*
 * here we provide for using hamlib to control an attached transceiver
 *
 * this is usually used in conjunction with alsa/pulse/sound_io...
 *
 * ft8goblin and sigcapd both use this, to allow tunable source/sinks via hamlib
 */
#include "config.h"
#include <hamlib/amplifier.h>
#include <hamlib/rig.h>
#include <hamlib/rotator.h>

//
// We have to be careful to ensure all data is wrapped in a Hamlib object...
// This is the only way we can support multiple radios.
//
