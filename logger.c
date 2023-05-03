#include "config.h"
#include "logger.h"

// from main program usually
extern char *progname;
extern int dying;

LogHndl *mainlog = NULL;

static struct log_levels {
   const char *str;
   int level;
} log_levels[] = {
   { "debug", LOG_DEBUG },
   { "info", LOG_INFO },
   { "notice", LOG_NOTICE },
   { "warning", LOG_WARNING },
   { "error", LOG_ERR },
   { "critical", LOG_CRIT },
   { "alert", LOG_ALERT },
   { "emergency", LOG_EMERG },
   { NULL, -1 },
};

static inline const char *LogName(int level) {
   struct log_levels *lp = log_levels;

   do {
      if (lp->level == level)
         return lp->str;

      lp++;
   } while(lp->str != NULL);

   return LOG_INVALID_STR;
}

static inline const int LogLevel(const char *name) {
   struct log_levels *lp = log_levels;

   do {
      if (strcasecmp(lp->str, name) == 0)
         return lp->level;

      lp++;
   } while(lp->str != NULL);

   return -1;
}

int log_send_va(LogHndl *log, int level, const char *msg, va_list ap) {
   char datebuf[32];
   char buf[4096];
   int max;
   time_t now = time(NULL);
   FILE *fp = stderr;		// default to stderr til logging is up

   // create data stamp
   memset(datebuf, 0, sizeof(datebuf));
   strftime(datebuf, sizeof(datebuf) - 1, "%Y-%m-%d %H:%M:%S", localtime(&now));

   max = LogLevel(cfg_get_str(cfg, "logging/ui-loglevel"));

   if (max < level)
      return -1;


   if (log) {
      if (log->type == LOG_syslog) {
         vsyslog(level, msg, ap);
      } else if (log->type != NONE) {
         memset(buf, 0, 4096);
         vsnprintf(buf, 4095, msg, ap);
      }

      if (log->type != LOG_stderr && log->fp)
         fp = log->fp;
   }

   // if we're dying, try to write to stderr...
   if (dying) {
      fprintf(stderr, "%s [%s] %s\n", datebuf, LogName(level), buf);
      fflush(stderr);
   }

   fprintf(fp, "%s [%s] %s\n", datebuf, LogName(level), buf);

   // force it to disk
   fflush(fp);

   va_end(ap);
   return 0;
}

int log_send(LogHndl *log, int level, const char *msg, ...) {
   va_list ap;
   va_start(ap, msg);
   return log_send_va(log, level, msg, ap);
}


LogHndl *log_open(const char *path) {
   LogHndl *log = (LogHndl *)malloc(sizeof(LogHndl));
   if (log == NULL) {
      fprintf(stderr, "log_init: out of memory!\n");
      exit(ENOMEM);
   }

   if (strcasecmp(path, "syslog") == 0) {
      log->type = LOG_syslog;
      openlog(progname, LOG_NDELAY|LOG_PID, LOG_DAEMON);
   } else if (strcasecmp(path, "stderr") == 0) {
      log->type = LOG_stderr;
      log->fp = stderr;
   } else if (strncasecmp(path, "fifo://", 7) == 0) {
      if (is_fifo(path + 7) || is_file(path + 7))
         unlink(path + 7);

      mkfifo(path+7, 0600);

      if (!(log->fp = fopen(path + 7, "w"))) {
         fprintf(stderr, "Failed opening log fifo '%s' %s (%d)", path+7, errno, strerror(errno));
         log->fp = stderr;
      } else
         log->type = LOG_fifo;
   } else if (strncasecmp(path, "file://", 7) == 0) {
      if (!(log->fp = fopen(path + 7, "w+"))) {
         fprintf(stderr, "failed opening log file '%s' %s (%d)", path+7, errno, strerror(errno));
         log->fp = stderr;
      } else
         log->type = LOG_file;
   }

   return log;
}

void log_destroy(LogHndl *log) {
   if (log == NULL)
      return;

   if (log->type == LOG_file || log->type == LOG_fifo) {
      fflush(log->fp);
      fclose(log->fp);
   } else if (log->type == LOG_syslog) {
      closelog();
   }

   free(log);
   log = NULL;
}
