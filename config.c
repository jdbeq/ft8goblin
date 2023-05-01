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
    unsigned char *fileData;
    size_t rd;
    char errbuf[1024];
    yajl_val node = NULL;
    struct stat sb;
    FILE *fp = NULL;
    size_t cfg_len = -1;
    fileData = NULL;
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
    if ((fileData = malloc(cfg_len)) == NULL) {
       fprintf(stderr, "parse_config: error allocating memory for config parser, exiting!\n");
       exit(255);
    }

    if ((fp = fopen(cfgfile, "r")) == NULL) {
       fprintf(stderr, "parse_config: fopen(%s, \"r\") failed: %d: %s\n", cfgfile, errno, strerror(errno));
       free(fileData);
       fclose(fp);
       return NULL;
    }

    /* read the entire config file */
    rd = fread((void *) fileData, 1, cfg_len, fp);
    // XXX: this needs improved to allow multiple fread() calls, if needed.. though config really shouldn't ever get that big....

    /* file read error handling */
    if (rd == 0 && !feof(fp)) {
       fprintf(stderr, "%s: error encountered on file read\n", __FUNCTION__);
       free(fileData);
       fclose(fp);
       return NULL;
    } else if (rd < cfg_len) {
       fprintf(stderr, "%s: config file too big\n", __FUNCTION__);
       free(fileData);
       fclose(fp);
       return NULL;
    }

    // close the file as we no longer need it...
    fclose(fp);
    fp = NULL;

    /* we have the whole config file in memory.  let's parse it ... */
    node = yajl_tree_parse((const char *) fileData, errbuf, sizeof(errbuf));

    /* parse error handling */
    if (node == NULL) {
        fprintf(stderr, "%s<%s>: parse_error: ", __FUNCTION__, cfgfile);

        if (strlen(errbuf))
           fprintf(stderr, " %s", errbuf);
        else
           fprintf(stderr, "unknown error");

        fprintf(stderr, "\n");

        free(fileData);
        return NULL;
    }

    free(fileData);
    return node;
}

int free_config(yajl_val node) {
    yajl_tree_free(node);
}

//
// Try to find a configuration file and load it!
//
yajl_val load_config(void) {
   yajl_val rv = NULL;

   // Try current directory first
   if ((rv = parse_config("config.json")) != NULL)
      return rv;

   // and then the global directory, if config in pwd wasn't found...
   if (rv == NULL && (rv = parse_config("/etc/ft8md/config.json")) != NULL)
      return rv;

   // nope, return error
   return NULL;
}

char **cfg_mkpath(const char *path) {
   char **rpath;

   // split the path on / character
   return rpath;
}

int cfg_get_int(yajl_val cfg, const char *path) {
    int rv = -1;

    // rpath should be constructed by calling cfg_mkpath()
    const char *rpath[] = { "ui", "autoscroll", (const char *) 0 };
    yajl_val v = yajl_tree_get(cfg, rpath, yajl_t_number);

    if (v) {
       rv = YAJL_GET_INTEGER(v);
       fprintf(stderr, "%s: <%s> = %d\n", __FUNCTION__, path, rv);
    } else {
       fprintf(stderr, "%s: no such node: %s\n", __FUNCTION__, path[0]);
    }

    return rv;
}

const char *cfg_get_str(yajl_val cfg, const char *path) {
    const char *rv = NULL;

    // rpath should be constructed by calling cfg_mkpath()
    const char *rpath[] = { "ui", "autoscroll", (const char *) 0 };
    yajl_val v = yajl_tree_get(cfg, rpath, yajl_t_string);

    if (v) {
       rv = YAJL_GET_STRING(v);
       fprintf(stderr, "%s: <%s> = %s\n", __FUNCTION__, path, rv);
    } else {
       fprintf(stderr, "%s: no such node: %s\n", __FUNCTION__, path[0]);
    }

    return rv;
}
