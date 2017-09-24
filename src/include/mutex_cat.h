/* mutex_cat.h */

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

#define _GNU_
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdbool.h>

char * make_string(char const * const fmt, ...);

void clean_and_exit(char * const message, char const * const istream_name, char const * const ostream_name, FILE const * istream_fp, FILE const * ostream_fp, int const lock_fd, bool had_erro);

int serialise(FILE const * const istream_fp, FILE const * const ostream_fp, int const lock_fd, char const * message);
