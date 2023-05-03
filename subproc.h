#if	!defined(_subproc_h)
#define	_subproc_h

#define	MAX_SUBPROC	64
#ifdef __cplusplus
extern "C" {
#endif
   typedef struct subproc subproc_t;
   struct subproc {
      int 		pid;			// host process id
      char		name[64];		// subprocess name (for subproc_list() etc)
   };
   extern int subproc_killall(int signum);
   extern void subproc_shutdown(void);

#ifdef __cplusplus
};
#endif
#endif	// !defined(_subproc_h)
