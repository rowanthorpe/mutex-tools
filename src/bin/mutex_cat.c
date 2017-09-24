/* mutex_cat.c */

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

#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include "mutex_cat.h"

int main(int argc, char * * argv)
{
  FILE const * istream_fp = NULL;
  FILE const * ostream_fp = NULL;
  int lock_fd = -1;
  char const * lock_name = "";
  char const * istream_name = "";
  char const * ostream_name = "";
  char * message = NULL;

  if (argc != 4) {
    message = make_string("Wrong number of arguments.\n");
    clean_and_exit(message, istream_name, ostream_name, istream_fp, ostream_fp, lock_fd, true);
  }
  lock_name = argv[1];
  istream_name = argv[2];
  ostream_name = argv[3];
  if ((lock_fd = open(lock_name, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) == -1) {
    message = make_string("Cannot open \"%s\" for use as lockfile.\n", lock_name);
    clean_and_exit(message, istream_name, ostream_name, istream_fp, ostream_fp, lock_fd, true);
  }
  if (strncmp(istream_name, "-", 2) == 0)
    istream_fp = stdin;
  else if ((istream_fp = fopen(istream_name, "r")) == NULL) {
    message = make_string("Cannot open \"%s\" for use as input.\n", istream_name);
    clean_and_exit(message, istream_name, ostream_name, istream_fp, ostream_fp, lock_fd, true);
  }
  if (strncmp(ostream_name, "-", 2) == 0)
    ostream_fp = stdout;
  else if (strncmp(ostream_name, "=", 2) == 0)
    ostream_fp = stderr;
  else if ((ostream_fp = fopen(ostream_name, "w")) == NULL) {
    message = make_string("Cannot open \"%s\" for use as output.\n", ostream_name);
    clean_and_exit(message, istream_name, ostream_name, istream_fp, ostream_fp, lock_fd, true);
  }
  if (serialise(istream_fp, ostream_fp, lock_fd, message) == EXIT_FAILURE)
    clean_and_exit(message, istream_name, ostream_name, istream_fp, ostream_fp, lock_fd, true);
  clean_and_exit(message, istream_name, ostream_name, istream_fp, ostream_fp, lock_fd, false);
  exit(EXIT_FAILURE); /* should never get here... */
}
