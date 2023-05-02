/*
 * Support for looking up callsigns via QRZ XML API.
 *
 * This is only useful for paid QRZ members.
 *
 * XXX: We should cache results in sqlite so lookups only happen once a day.
 */
#include "config.h"
#include "qrz-xml.h"
#include "sql.h"
