/*
 * subprocess management:
 *	Here we deal with keeping subprocesses alive.
 * Steps (performed on all subprocesses)
 *	* Start process
 *	* Watch for process to exit
 *	* If zero (success) exit status, immediately restart
 *	* If non-zero (failure) exit status, delay randomly up to 15 seconds before restarting process
 *	* If a process has crashed more than cfg:supervisor/max-crashes in the last cfg:supervisor/max-crash-time then don't bother respawning it..
 */
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <ctype.h>
#include <string.h>
#include <sys/wait.h>
#include <termbox2.h>
#include "subproc.h"
#include "ui.h"
#include "logger.h"

extern TextArea *msgbox;
extern int y;			// from ui.c
static int max_subprocess = 0;

// this shouldn't be exported as we'll soon provide utility functions for it
static subproc_t *children[MAX_SUBPROC];

static void subproc_delete(int i) {
   if (children[i] == NULL) {
      ta_printf(msgbox, "$RED$subproc_delete: invalid child process %i (NULL) requested for deletion", i);
      return;
   }

   if (max_subprocess > 0)
      max_subprocess--;

   free(children[i]);
   children[i] = NULL;
}

int subproc_killall(int signum) {
   char *signame = NULL;
   int rv = 0;

   if (signum == SIGTERM) {
      signame = "SIGTERM";
   } else if (signum == SIGKILL) {
      signame = "SIGKILL";
   } else if (signum == SIGHUP) {
      signame = "SIGHUP";
   } else if (signum == SIGUSR1) {
      signame = "SIGUSR1";
   } else if (signum == SIGUSR2) {
      signame = "SIGUSR2";
   } else {
      signame = "INVALID";
   }

   if (max_subprocess > MAX_SUBPROC) {
      ta_printf(msgbox, "$RED$subproc_killall: max_subprocess (%d) > MAX_SUBPROC (%d), this is wrong!", max_subprocess, MAX_SUBPROC);
      log_send(mainlog, LOG_CRIT, "subproc_killall: max_subprocess (%d) > MAX_SUBPROC (%d), this is wrong!", max_subprocess, MAX_SUBPROC);
      tb_present();
      exit(200);
   }

   int i = 0;
   for (i = 0; i < max_subprocess; i++) {
      ta_printf(msgbox, "$YELLOW$sending %s (%d) to child process %s <%d>...", signame, signum, children[i]->name, children[i]->pid);
      log_send(mainlog, LOG_NOTICE, "sending %s (%d) to child process %s <%d>...", signame, signum, children[i]->name, children[i]->pid);
      tb_present();

      // if successfully sent signal, increment rv, so we'll sleep if called from subproc_shutdown()
      if (kill(children[i]->pid, signum) == 0)
         rv++;

      time_t wstart = time(NULL);
      int wstatus;
      int rv = waitpid(children[i]->pid, &wstatus, WNOHANG);

      if (rv == -1) {
        // an error occured
        continue;
      } else if (rv == 0) {
        // it didn't exit yet...
        ta_printf(msgbox, "$YELLOW$-- no response, sleeping 3 seconds before next attempt...");
        sleep(3);
        continue;
      } else {
        // we can delete the subproc and avoid sending further signals to a dead PID...
        subproc_delete(i);
      }
   }

   // delete the data structure
   subproc_delete(i);
   // return > 0, if we sent any kill signals
   return rv;
}

// Shut down subprocesses and throw a message to the console to let the operator know what's happening
void subproc_shutdown_all(void) {
   if (subproc_killall(SIGTERM) > 0)
      if (subproc_check_all() > 0)
         sleep(2);

   if (subproc_killall(SIGKILL) > 0)
      if (subproc_check_all() > 0)
         sleep(1);
}

static time_t get_random_interval(int min, int max) {
   unsigned int  seed;
   time_t rs_time;
   seed = arc4random();
   srand(seed);
   rs_time = (rand() % (max - min) + min);

   return rs_time;
}

// Check all subprocesses to make sure they're all still alive
int subproc_check_all(void) {
   int rv = 0;

   // scan the child process table and see if any have died
   for (int i = 0; i < max_subprocess; i++) {
      // check if the process is still alive
      time_t wstart = time(NULL);
      int wstatus;
      int pid = waitpid(children[i]->pid, &wstatus, WNOHANG);

      if (pid == -1) {
        // an error occured
        log_send(mainlog, LOG_DEBUG, "subproc_check_all: waitpid on subprocess %d returned %d (%s)", i, errno, strerror(errno));
        continue;
      } else if (pid == 0) {
        // process is still alive, count it
        rv++;
        continue;
      } else if (children[i]->pid == pid) {
        // process has exited
        log_send(mainlog, LOG_CRIT, "subprocess %d (%s) exited, registering it for restart in %lu seconds", i, children[i]->name, get_random_interval(3, 15));
        // mark the PID as invalid and needs_restarted
        children[i]->pid = -1;
        children[i]->needs_restarted = 1;
        int watchdog_expire = cfg_get_int(cfg, "supervisor/max-crash-time");
        int watchdog_max_events = cfg_get_int(cfg, "supervisor/max-crashes");

        // are we already performing watchdog on this subproc due to previous crashes??
        if (children[i]->watchdog_start == 0) {
           // nope, start the watchdog
           children[i]->watchdog_start = now;
           children[i]->watchdog_events = 0;
        } else if (children[i]->watchdog_start > 0) {
           // has the watchdog expired?
           if (children[i]->watchdog_start + watchdog_expire <= now) {
              // has the 
              if (children[i]->watchdog_events < watchdog_max_events) {
                 log_send(mainlog, LOG_NOTICE, "subprocess %d (%s) has restored normal operation. It crashed %d times in %lu seconds.", i, children[i]->name, children[i]->watchdog_events, (now - children[i]->watchdog_start));
                 children[i]->watchdog_start = 0;
                 children[i]->watchdog_events = 0;
              } else {
                 log_send(mainlog, LOG_CRIT, "subprocess %d (%s) has crashed %d times in %lu seconds. disabling restarts", i, children[i]->name, children[i]->watchdog_events, (now - children[i]->watchdog_start));
              }

              // either way, we don't need a restart...
              children[i]->needs_restarted = 0;
              children[i]->restart_time = 0;
           }
        } else { // no watchdog is still active
           children[i]->needs_restarted = 1;
           log_send(mainlog, LOG_DEBUG, "subprocess %d (%s) has crashed. This is the %d time in %lu seconds. It will be disabled after %d times.", i, children[i]->name, children[i]->watchdog_events, (now - children[i]->watchdog_start), watchdog_max_events);
        }

        // schedule 3-15 seconds in the future..
        children[i]->restart_time = get_random_interval(3, 15) + now;
      } else {
        log_send(mainlog, LOG_DEBUG, "unexpected return valid %d from waitpid(%d) for subproc %d", pid, children[i]->pid, i);
      }
   }
   return rv;
}
