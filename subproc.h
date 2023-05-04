#if	!defined(_subproc_h)
#define	_subproc_h
#include <limits.h>
#include <stdlib.h>

#define	MAX_SUBPROC	64
#ifdef __cplusplus
extern "C" {
#endif
   typedef struct subproc subproc_t;
   struct subproc {
      int 		pid;			// host process id
      char		name[64];		// subprocess name (for subproc_list() etc)
      char		*cmd_line[128];		// pointer to the arguments needed to execute the process
      time_t		restart_time;		// when should we restart?
      time_t		watchdog_start;		// watchdog is started if the process crashes, it expires after cfg:supervisor/max-crash-time.
                                                // when active, watchdog_events tracks the number of crashes
      int		watchdog_events;	// during watchdog, this is incremented with the number of crashes
      int		needs_restarted;	// does it need restarted by the periodic thread?
   };
   extern int subproc_killall(int signum);
   extern void subproc_shutdown_all(void);
   extern int subproc_check_all(void);
   extern int subproc_respawn_corpses(void);
#ifdef __cplusplus
};
#endif
#endif	// !defined(_subproc_h)
