#if	!defined(_subproc_h)
#define	_subproc_h

#define	MAX_SUBPROC	64

typedef struct subproc subproc_t;
struct subproc {
   int 		pid;			// host process id
   char		name[64];		// subprocess name (for subproc_list() etc)
};
extern void subproc_killall(int signum);
extern void subproc_shutdown(void);

#endif	// !defined(_subproc_h)
