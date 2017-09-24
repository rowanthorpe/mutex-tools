/* libmutex_cat.c */

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

#include <stdlib.h>
#include <sys/file.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include "mutex_cat.h"

char * make_string(char const * const fmt, ...)
{
  int size = 0;
  char * p = NULL;
  va_list ap;

  va_start(ap, fmt);
  size = vsnprintf(NULL, 0, fmt, ap);
  va_end(ap);
  if (size < 0)
    return NULL;
  size++;
  p = malloc((size_t)size);
  if (p == NULL)
    return NULL;
  va_start(ap, fmt);
  size = vsnprintf(p, (size_t)size, fmt, ap);
  if (size < 0) {
    free(p);
    return NULL;
  }
  va_end(ap);
  return p;
}

void clean_and_exit(char * const message, char const * const istream_name, char const * const ostream_name, FILE const * istream_fp, FILE const * ostream_fp, int const lock_fd, bool const had_error)
{
  if (message) {
    if (message[0])
      fprintf(stderr, message);
    free(message);
  }
  if (istream_fp && strncmp(istream_name, "-", 2) != 0)
    fclose((FILE *)istream_fp);
  if (ostream_fp && strncmp(ostream_name, "-", 2) != 0)
    fclose((FILE *)ostream_fp);
  if (had_error) {
    if (lock_fd != -1) {
      lseek(lock_fd, 0, SEEK_SET);
      ftruncate(lock_fd, 0);
      dprintf(lock_fd, "%d", getpid());
      close(lock_fd);
    }
    exit(EXIT_FAILURE);
  } else
    exit(EXIT_SUCCESS);
}

int serialise(FILE const * const istream_fp, FILE const * const ostream_fp, int const lock_fd, char const * message)
{
  char * line = NULL;
  size_t linelen = 0;

  while (getline(&line, &linelen, (FILE *)istream_fp) != -1) {
    if (flock(lock_fd, LOCK_EX) == -1)
      message = make_string("Failed attempting to lock lockfile.\n");
    if (fprintf((FILE *)ostream_fp, "%s", line) < 0)
      message = make_string("Failed attempting to write to output stream.\n");
    if (flock(lock_fd, LOCK_UN) == -1)
      message = make_string("Failed attempting to unlock lockfile.\n");
    free(line);
    line = NULL;
    linelen = 0;
    if (message)
      return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
