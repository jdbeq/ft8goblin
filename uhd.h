//
// Interface with libuhd for USRP devices
//
#if	!defined(_uhd_h)
#define _uhd_h
#include <uhd.h>

#ifdef __cplusplus
extern "C" {
#endif
   extern int uhd_connect(void);

#ifdef __cplusplus
};
#endif

#endif	// !defined(_uhd_h)
