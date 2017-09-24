#!/bin/sh

#  © Copyright 2017 Rowan Thorpe
#
#  This file is part of mutex-tools
#
#  mutex-tools is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  mutex-tools is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with mutex-tools. If not, see <http://www.gnu.org/licenses/>.

if test -z "${MUTEX_TOOLS_TESTING}"; then
    MUTEX_TOOLS_TESTING=1
    mutex=''
    trap 'test -z "${mutex}" || rm -f "${mutex}"' EXIT
    mutex="$(mktemp)" || { printf 'mutex_cat test failed to create mutex tempfile\n' >&2; exit 1; }
    . "${0}"
    { a & b & wait; } | grep 'a' | grep -q 'b' || { printf 'mutex_cat control test didn'\''t even interpolate\n' >&2; exit 1; }
    { a & b >&2 & wait; } 2>&1 | grep 'a' | grep -q 'b' || { printf 'mutex_cat control test didn'\''t even interpolate\n' >&2; exit 1; }
    ! { a | mutex_cat "${mutex}" - - & b | mutex_cat "${mutex}" - - & wait; } | grep 'a' | grep -q 'b' || { printf 'mutex_cat test failed to prevent interpolation\n' >&2; exit 1; }
    ! { a | mutex_cat "${mutex}" - - & b | mutex_cat "${mutex}" - - >&2 & wait; } 2>&1 | grep 'a' | grep -q 'b' || { printf 'mutex_cat test failed to prevent interpolation\n' >&2; exit 1; }
else
    a() {
        for x in $(seq 10); do
            for x in $(seq 50); do
                printf a
                /bin/sleep 0.001
            done
            printf \\n
        done
    }
    b() {
        for x in $(seq 10); do
            for x in $(seq 50); do
                printf b
                /bin/sleep 0.001
            done
            printf \\n
        done
    }
fi
