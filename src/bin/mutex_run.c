/* mutex_run.c */

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
#include "mutex_run.h"

int main(int argc, char * * argv)
{
  if (argc < 2) {
    fprintf(stderr, "Wrong number of arguments.\n");
    exit(EXIT_FAILURE);
  }
  exit(run(argv[1], (char const * const * const)&argv[1]));
}
