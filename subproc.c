#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <ctype.h>
#include <string.h>
#include <sys/wait.h>
#include "config.h"
#include "termbox2/termbox2.h"
#include "subproc.h"

extern int y;			// from ui.c
static int max_subprocess = 0;

// this shouldn't be exported as we'll soon provide utility functions for it
static subproc_t *children[MAX_SUBPROC];

static void subproc_delete(int i) {
   if (children[i] == NULL) {
      tb_printf(0, y++, TB_RED|TB_BOLD, 0, "%s: invalid child process %i (NULL) requested for deletion", __FUNCTION__, i);
      return;
   }

   free(children[i]);
   children[i] = NULL;
}
void subproc_killall(int signum) {
   char *signame = NULL;

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
      tb_printf(0, y++, TB_RED|TB_BOLD, 0, "%s: max_subprocess (%d) > MAX_SUBPROC (%d), this is wrong!", __FUNCTION__, max_subprocess, MAX_SUBPROC);
      tb_present();
      exit(200);
   }

   int i = 0;
   for (i = 0; i < max_subprocess; i++) {
      tb_printf(0, y++, TB_YELLOW|TB_BOLD, 0, "sending %s (%d) to child process %s <%d>...", signame, signum, children[i]->name, children[i]->pid);
      tb_present();
      kill(children[i]->pid, signum);

      time_t wstart = time(NULL);
      int wstatus;
      int rv = waitpid(children[i]->pid, &wstatus, WNOHANG);

      if (rv == -1) {
        // an error occured
        continue;
      } else if (rv == 0) {
        // it didn't exit yet...
//        tb_printf(0, y++, TB_YELLOW|TB_BOLD, 0, "-- no response, sleeping 3 seconds before next attempt...");
//        sleep(3);
        continue;
      } else {
        // we can delete the subproc and avoid sending further signals to a dead PID...
        subproc_delete(i);
      }
   }

   // delete the data structure
   subproc_delete(i);
}

// Shut down subprocesses and throw a message to the console to let the operator know what's happening
void subproc_shutdown(void) {
   y = 1;
   subproc_killall(SIGTERM);
   sleep(2);
   subproc_killall(SIGKILL);
   sleep(1);
}
