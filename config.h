#if	!defined(_config_h)
#define	_config_h
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <yajl/yajl_tree.h>
#include <sys/stat.h>
#include <errno.h>

// Global configuration pointer
extern yajl_val cfg;

// Parse the configuration and return a yajl tree
extern yajl_val parse_config(const char *cfgfile);

extern int free_config(yajl_val node);
extern yajl_val load_config(void);
extern int cfg_get_int(yajl_val cfg, const char *path);
extern const char *cfg_get_str(yajl_val cfg, const char *path);

#endif	// !defined(_config_h)
