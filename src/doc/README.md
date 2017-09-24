mutex-tools
===========

Description
-----------

This package includes two executable tools written in c:

1. 'mutex_cat' which acts like a simplified 'cat' but holds a mutex whenever
   outputing a line. This enables multiple instances outputting to the same
   stream to avoid interlacing their output by using the same mutex.
1. 'mutex_run' which uses 'mutex_cat' functionality internally in order to
   be used as a wrapper for an executable which prevents the stdout and
   stderr from being interlaced.

and their functionality also in libs for including in your own c source.

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
  them dynamically either tweak the build-tool `m` or compile them manually.

Authors
-------

Rowan Thorpe <rowan@rowanthorpe.com>

License
-------

mutex-tools uses the GPL3+ license, check COPYING file.

Example usage
-------------

Examples of using the executable tools are:

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

or look at the test-scripts at `src/test/mutex_run-test.sh` and
`src/test/mutex_cat-test.sh` for more enlightenment.

As examples of using the libs, the easiest is to look at the source for the
executable files at `src/bin/mutex_run` and `src/bin/mutex_cat`

FIXME
-----

* Nothing presently

TODO
----

* `--help` output for both tools.
* Add config-var to `libmutex_cat` and associated flag to `mutex_cat` to
  change the delimiter to something other than newline (`getdelim()` instead
  of `getline()`)
* Add config-var to `libmutex_cat` and associated flag to `mutex_cat` to
  activate holding the mutex until the process closes (i.e. until EOF instead
  of each EOL).
* Reach feature/flag-parity with "GNU cat", then propose `mutex_cat`
  functionality as a "GNU extension" to that, perhaps under the flags
  `-m|--mutex-till-eol` and `-M|--mutex-till-eof`.
