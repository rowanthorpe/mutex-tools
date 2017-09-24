mutex-tools
===========

Description
-----------

This package includes two tools:

1. 'mutex_cat' which acts like a simplified 'cat' but holds a mutex whenever
   outputing a line. This enables multiple instances outputting to the same
   stream to avoid interlacing their output by using the same mutex.
1. 'mutex_run' which uses 'mutex_cat' functionality internally in order to
   be used as a wrapper for an executable which prevents the stdout and
   stderr from being interlaced.

and equivalent c libs for including in your own tools.

Building & Installation
-----------------------

Run ./m help for details about using "m" for building. Generated tarballs are
appropriate for installing under a $PREFIX like /usr.

Built Components
----------------

When built with `./m devel` or `./m maintainer`:

* bin/
    * mutex_cat
    * mutex_run
* include/
    * mutex_cat/
    * mutex_cat/mutex_cat.h
    * mutex_cat/mutex_run.h
* lib/
    * libmutex_cat.a
    * libmutex_run.a
* share/
    * doc/
        * COPYING
        * README.html
    * mutex-tools/
        * test/
            * mutex_cat-test.sh
            * mutex_run-test.sh

NOTA BENE
---------

* If fed long enough lines (or files, in EOF-mode) `mutex_cat` might freeze the
  connected pipelines, if the programs at the other end fill their buffers and
  can't continue.
* By default the executables and libs are compiled with `-static`, to compile
  them dynamically either tweak `m` or compile them manually.

Authors
-------

Rowan Thorpe <rowan@rowanthorpe.com>

License
-------

mutex-tools uses the GPL3+ license, check COPYING file.

Example usage
-------------

```sh
some_complex_output_generator | mutex_cat mutex-name.lock - - &
another_complex_output_generator | mutex_cat mutex-name.lock - - &
wait
```

```sh
mutex_cat my-mutex.lck input-pipe my-logfile.log &
mutex_cat my-mutex.lck another-input-pipe my-logfile.log &
wait
```

```sh
mutex_run concurrent-stdout-and-stderr-generator
```

As examples of using the libs, the easiest is to look at the below which are
just copy-pastes of the `mutex_cat` and `mutex_run` source.

```c
/* mutex_run.c */

#include <stdio.h>
#include <stdlib.h>
#include <mutex_run.h>

int main(int argc, char * * argv)
{
  if (argc < 2) {
    fprintf(stderr, "Wrong number of arguments.\n");
    exit(EXIT_FAILURE);
  }
  exit(run(argv[1], (char const * const * const)&argv[1]));
}
```

```c
/* mutex_cat.c */

#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <mutex_cat.h>

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
```

FIXME
-----

* Nothing presently

TODO
----

* `--help` output for both tools.
* Add config-var to `libmutex_cat` and associated flag to `mutex_cat` to
  change the delimiter to something other than newline (`getdelim() instead
  of `getline()`)
* Add config-var to `libmutex_cat` and associated flag to `mutex_cat` to
  activate holding the mutex until the process closes (i.e. until EOF instead
  of each EOL).
* Reach feature/flag-parity with "GNU cat", then propose `mutex_cat`
  functionality as a "GNU extension" to that, perhaps under the flags
  `-m|--mutex-till-eol` and `-M|--mutex-till-eof`.
