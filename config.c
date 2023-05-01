/*
 * json configuration parser
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <yajl/yajl_tree.h>
#include <sys/stat.h>
#include <errno.h>
#include "config.h"

yajl_val cfg = NULL;

yajl_val parse_config(const char *cfgfile) {
    FILE *fp = NULL;
    unsigned char *data = NULL;
    size_t rd;
    char errbuf[1024];
    yajl_val node = NULL;
    struct stat sb;
    size_t cfg_len = -1;
    errbuf[0] = 0;

    if (cfgfile == NULL) {
       fprintf(stderr, "parse_config: NULL cfgfile is not sane, exiting!\n");
       exit(255);
    }

    if (lstat(cfgfile, &sb) == -1) {
       fprintf(stderr, "parse_config: error lstat()ing cfgfile (%s): %d: %s\n", cfgfile, errno, strerror(errno));
       return NULL;
    }

    // this is gross ;)
    cfg_len = sb.st_size;
    if ((data = malloc(cfg_len)) == NULL) {
       fprintf(stderr, "parse_config: error allocating memory for config parser, exiting!\n");
       exit(255);
    }

    if ((fp = fopen(cfgfile, "r")) == NULL) {
       fprintf(stderr, "parse_config: fopen(%s, \"r\") failed: %d: %s\n", cfgfile, errno, strerror(errno));
       free(data);
       fclose(fp);
       return NULL;
    }

    /* read the entire config file */
    rd = fread((void *) data, 1, cfg_len, fp);
    // XXX: this needs improved to allow multiple fread() calls, if needed.. though config really shouldn't ever get that big....

    /* file read error handling */
    if (rd == 0 && !feof(fp)) {
       fprintf(stderr, "%s: error encountered on file read\n", __FUNCTION__);
       free(data);
       fclose(fp);
       return NULL;
    } else if (rd < cfg_len) {
       fprintf(stderr, "%s: config file too big\n", __FUNCTION__);
       free(data);
       fclose(fp);
       return NULL;
    }

    // close the file as we no longer need it...
    fclose(fp);
    fp = NULL;

    /* we have the whole config file in memory.  let's parse it ... */
    node = yajl_tree_parse((const char *) data, errbuf, sizeof(errbuf));

    /* parse error handling */
    if (node == NULL) {
        fprintf(stderr, "%s<%s>: parse_error: ", __FUNCTION__, cfgfile);

        if (strlen(errbuf))
           fprintf(stderr, " %s", errbuf);
        else
           fprintf(stderr, "unknown error");

        fprintf(stderr, "\n");

        free(data);
        return NULL;
    }

    free(data);
    return node;
}

int free_config(yajl_val node) {
    yajl_tree_free(node);

    return 0;
}

//
// Try to find a configuration file and load it!
//
yajl_val load_config(void) {
   yajl_val rv = NULL;

   // Try current directory first
   if ((rv = parse_config("config.json")) != NULL) {
      cfg = rv;
      return rv;
   }

   // and then the global directory, if config in pwd wasn't found...
   if (rv == NULL && (rv = parse_config("/etc/ft8md/config.json")) != NULL)
      return rv;

   // nope, return error
   return NULL;
}

// This function shouldn't be exported
static char **cfg_mkpath(char *path, char **rpath) {
   if (rpath == NULL) {
      fprintf(stderr, "%s: rpath MUST not be NULL\n", __FUNCTION__);
      return NULL;
   }

   if (path == NULL) {
      fprintf(stderr, "%s: calling with a NULL path makes no sense!\n", __FUNCTION__);
      return NULL;
   }

   // zero out the array
   memset(rpath, 0, sizeof(rpath));

   char *restrict rp = path;
   char *p = NULL;
   int ai = 0;

   if ((p = strtok_r(path, "/.", &rp)) == NULL) {
      fprintf(stderr, "%s: error parsing path %s\n", __FUNCTION__, path);
      return NULL;
   }
   rpath[ai] = p;
   ai++;

   while (1) {
      if ((p = strtok_r(NULL, "/.", &rp)) == NULL) {
         break;
      }
      rpath[ai] = p;
      ai++;
   }

   // make sure last item is NULL
   rpath[ai] = NULL;
   return rpath;
}

int cfg_get_int(yajl_val cfg, const char *path) {
   int rv = -1;

   if (path == NULL) {
      fprintf(stderr, "%s called with path == NULL, bailing!", __FUNCTION__);
      return rv;
   }

   // rpath should be constructed by calling cfg_mkpath()
   char *rpath[PATHMAX_JSON];
   char *cpath = strdup(path);
   cfg_mkpath(cpath, rpath);
   yajl_val v = yajl_tree_get(cfg, (const char **)rpath, yajl_t_number);
   free(cpath);

   if (v) {
      rv = YAJL_GET_INTEGER(v);
//      fprintf(stderr, "%s: <%s> = %d\n", __FUNCTION__, path, rv);
   } else {
      fprintf(stderr, "%s: no such node: %s\n", __FUNCTION__, path);
   }

   return rv;
}

const char *cfg_get_str(yajl_val cfg, const char *path) {
   char *rv = NULL;

   // rpath should be constructed by calling cfg_mkpath
   char *rpath[PATHMAX_JSON];
   char *cpath = strdup(path);
   cfg_mkpath(cpath, rpath);
   yajl_val v = yajl_tree_get(cfg, (const char **)rpath, yajl_t_string);
   free(cpath);

   if (v) {
      rv = YAJL_GET_STRING(v);
//      fprintf(stderr, "%s: <%s> = %s\n", __FUNCTION__, path, rv);
   } else {
      fprintf(stderr, "%s: no such node: %s\n", __FUNCTION__, path);
   }

   return rv;
}
