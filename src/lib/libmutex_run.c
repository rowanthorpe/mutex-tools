/* libmutex_run.c */

/*
  © Copyright 2017 Rowan Thorpe

  This file is part of mutex-tools

  mutex-tools is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  mutex-tools is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with mutex-tools. If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include "mutex_cat.h"
#include "mutex_run.h"

int lock_fd = -1;
char * lock_name = NULL;

void cleanup(int const exit_type, char const * const msg, int const errnum)
{
  extern int lock_fd;
  extern char * lock_name;
  char * errmsg = NULL;

  if (msg && msg[0]) {
    if (errnum != 0) {
      errmsg = strerror(errnum);
      fprintf(stderr, "%s: %s\n", msg, errmsg);
      free(errmsg);
    } else
      fprintf(stderr, msg);
  }
  if (lock_fd != -1) {
    close(lock_fd);
    unlink(lock_name);
  }
  if (lock_name)
    free(lock_name);
  exit(exit_type);
}

void onintr(int num)
{
  num = 0;
  cleanup(EXIT_FAILURE, "Exiting on interrupt\n", num);
}

void sigtrap(int const sig)
{
  struct sigaction new;
  struct sigaction old;
  new.sa_handler = SIG_IGN;
  new.sa_flags = 0;
  sigaction(sig, &new, &old);
  if (old.sa_handler != SIG_IGN) {
    new.sa_handler = onintr;
    sigaction(sig, &new, &old);
  }
}

int run(char const * const command, char const * const * const args)
{
  extern int lock_fd;
  extern char * lock_name;
  char const * message = NULL;
  int e_pipefd[2], o_pipefd[2];
  pid_t cpid1, cpid2, cpid3;
  FILE const * e_pipefp;
  FILE const * o_pipefp;

  sigtrap(SIGINT);
  sigtrap(SIGTERM);
  sigtrap(SIGQUIT);
  if ((lock_name = make_string("%s/mutex_run_XXXXXX", P_tmpdir)) == NULL)
    cleanup(EXIT_FAILURE, "Failed to generate tempfile template\n", 0);

  if ((lock_fd = mkstemp(lock_name)) == -1)
    cleanup(EXIT_FAILURE, "Failed to open lockfile\n", 0);

  if ((pipe(e_pipefd) == -1) || (pipe(o_pipefd) == -1))
    cleanup(EXIT_FAILURE, "pipe", errno);

  cpid1 = fork();
  if (cpid1 == -1)
    cleanup(EXIT_FAILURE, "fork", errno);
  else if (cpid1 == 0) {
    close(e_pipefd[1]);
    if ((e_pipefp = fdopen(e_pipefd[0], "r")) == NULL)
      cleanup(EXIT_FAILURE, "fdopen", errno);
    serialise(e_pipefp, stdout, lock_fd, message);
  } else {
    close(e_pipefd[0]);
    cpid2 = fork();
    if (cpid2 == -1)
      cleanup(EXIT_FAILURE, "fork", errno);
    else if (cpid2 == 0) {
      close(o_pipefd[1]);
      if ((o_pipefp = fdopen(o_pipefd[0], "r")) == NULL)
        cleanup(EXIT_FAILURE, "fdopen", errno);
      serialise(o_pipefp, stdout, lock_fd, message);
    } else {
      close(o_pipefd[0]);
      cpid3 = fork();
      if (cpid3 == -1)
        cleanup(EXIT_FAILURE, "fork", errno);
      else if (cpid3 == 0) {
        if (!dup2(o_pipefd[1], 1) || !dup2(e_pipefd[1], 2) || !execvp(command, (char * const *)args))
          cleanup(EXIT_FAILURE, "system", errno);
      } else
        wait(NULL);
    }
  }
  cleanup(EXIT_SUCCESS, NULL, 0);
  return EXIT_FAILURE; /* silence a compiler warning with this */
}
