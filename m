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

# TODO: Do Makefile-based equivalent of this, then push to github

set -e

setup_vars() {
    components='mutex_cat:- mutex_run:mutex_cat'
    doc_components='README'
    license_components='COPYING'
    config_components="${project_name}"

    # -ansi...
    EXTRA_CC_ARGS="${EXTRA_CC_ARGS:--O3 -mavx2 -fomit-frame-pointer -static}"
    EXTRA_CC_ARGS_MAINTAINER="${EXTRA_CC_ARGS_MAINTAINER:--O0 -gdwarf -g3 -fvar-tracking -fvar-tracking-assignments -static}"

    CC='gcc'
    AR='ar'
    STRIP='strip'
    TAR='tar'
    CHMOD='chmod'
    FIND='find'
    MKDIR='mkdir'
    RMDIR='rmdir'
}

cp_if_exists() {
    #TODO: by optflag use find/xargs/sed instead of cp, to transform file content on-the-fly?
    eval "dest=\"\${${#}}\""
    for argnum in $(seq $(expr ${#} - 1)); do
        ! test -e "${1}" || eval "cp -ax \"\${${argnum}}\" \"\${dest}\""
    done
}

rm_if_exists() { rm -f "${@}" 2>/dev/null || :; }

run_if_exists() { for arg do sh "${arg}"; done; }

compile() {
    ${CC} -pipe -pedantic-errors -march=native \
        -Wall -Werror -Wextra -Wpedantic -Wstrict-prototypes -Wmissing-prototypes \
        -Wold-style-definition -Wconversion -Wshadow -Winit-self -Wfloat-equal -Wwrite-strings \
        ${EXTRA_CC_ARGS} -I src/include -L build/lib ${*}
}

chomp() { sed -e 's/ $//'; }

unchomp() { sed -e 's/$/ /'; }

get_name() { printf %s ${1} | cut -d: -f1; }

get_names() { for arg do get_name ${arg} | unchomp; done | chomp; }

get_deps() { printf %s ${1} | cut -d: -f2- | tr ',' ' '; }

####

do_help() {
    cat <<EOF
build.sh [help|
          clean|fullclean|
          minimal|standard|devel|maintainer|
          mkdirs|rmdirs|
          clean_{build,dist,tar}|
          build_{ob,lib,bin,test,doc,conf}|
          dist_{ob,lib,bin,test,doc,conf,include,license}|
          tar
         ] [...] [...]

  clean = ${actions_clean}

  fullclean = ${actions_fullclean}

  minimal = ${actions_minimal}

  standard = ${actions_standard}

  devel = ${actions_devel}

  maintainer = (EXTRA_CC_ARGS="${EXTRA_CC_ARGS_MAINTAINER}") ${actions_maintainer}

  * default = ${actions_default}
EOF
    exit 0
}

do_mkdirs() {
    ${MKDIR} -p \
        src/doc src/etc src/lib src/bin src/include \
        build/doc build/etc build/lib build/bin \
        dist/share/doc dist/etc dist/lib dist/bin dist/include/${project_name} dist/share/${project_name}/test
}

do_rmdirs() {
    ${RMDIR} \
        src/doc src/etc src/lib src/bin src/include src \
        build/doc build/etc build/lib build/bin build \
        dist/share/doc dist/etc dist/lib dist/bin dist/include/${project_name} dist/include \
        dist/share/${project_name}/test dist/share/${project_name} dist/share dist 2>/dev/null || :
}

do_clean_build() {
    for arg in ${doc_components}; do
        rm_if_exists build/doc/${arg}.html
    done
    for arg in ${config_components}; do
        rm_if_exists build/etc/${arg}.conf
    done
    for arg in $(get_names ${components}); do
        rm_if_exists build/lib/lib${arg}.o build/lib/lib${arg}.a build/bin/${arg}
    done
}

do_clean_dist() {
    for arg in ${doc_components}; do
        rm_if_exists dist/share/doc/${arg}.html dist/share/doc/${license_components}
    done
    for arg in ${config_components}; do
        rm_if_exists dist/etc/${arg}.conf
    done
    for arg in $(get_names ${components}); do
        rm_if_exists dist/lib/lib${arg}.a dist/bin/${arg} dist/include/${project_name}/${arg}.h dist/share/${project_name}/test/${arg}-test.sh
    done
}

do_clean_tar() { rm_if_exists ${project_name}.tar.gz; }

do_build_ob() { for arg in $(get_names ${components}); do compile -c src/lib/lib${arg}.c -o build/lib/lib${arg}.o; done; }

do_build_lib() { for arg in $(get_names ${components}); do ${AR} rcs build/lib/lib${arg}.a build/lib/lib${arg}.o; done; }

do_build_bin() {
    for arg in ${components}; do
        name=$(get_name ${arg})
        deps="$(get_deps ${arg})"
        compile -o build/bin/${name} src/bin/${name}.c -l ${name}$(for dep in ${deps}; do test "x${dep}" = 'x-' || printf -- ' -l %s' ${dep}; done)
        ${STRIP} -s build/bin/${name}
        ${CHMOD} +x build/bin/${name}
    done
}

do_build_test() { for arg in $(get_names ${components}); do run_if_exists src/test/${arg}-test.sh; done; }

do_build_doc() {
    for arg in ${doc_components}; do
        if test -e src/doc/${arg}.md; then
            printf '<!DOCTYPE html>\n<html><head><title>%s</title></head><body>\n' ${arg} >build/doc/${arg}.html
            markdown_py -o html5 <src/doc/${arg}.md >>build/doc/${arg}.html
            printf '\n</body></html>' >>build/doc/${arg}.html
        fi
    done
}

do_build_conf() { for arg in ${config_components}; do cp_if_exists src/etc/${arg}.conf build/etc/; done; }

do_dist_ob() { for arg in $(get_names ${components}); do cp_if_exists build/lib/lib${arg}.a dist/lib/; done; }

do_dist_lib() { for arg in $(get_names ${components}); do cp_if_exists build/lib/${project_name}/${arg} dist/lib/${project_name}/; done; }

do_dist_bin() { for arg in $(get_names ${components}); do cp_if_exists build/bin/${arg} dist/bin/; done; }

do_dist_test() { for arg in $(get_names ${components}); do cp_if_exists src/test/${arg}-test.sh dist/share/${project_name}/test/; done; }

do_dist_doc() { for arg in ${doc_components}; do cp_if_exists build/doc/${arg}.html dist/share/doc/; done; }

do_dist_conf() { for arg in ${conf_components}; do cp_if_exists build/etc/${arg}.conf dist/etc/; done; }

do_dist_include() { for arg in $(get_names ${components}); do cp_if_exists src/include/${arg}.h dist/include/${project_name}/; done; }

do_dist_license() { for arg in ${license_components}; do cp_if_exists src/${arg} dist/share/doc/; done; }

do_tar() {
    cd dist
    ${TAR} --no-recursion -c -z -f ../${project_name}.tar.gz $(
        ${FIND} . -type f | \
            while IFS= read -r filename; do
                filepart="${filename}"
                while : ; do
                    printf '%s\n' "${filepart}"
                    filepart="$(dirname "${filepart}")"
                    test -n "${filepart}" && ! test . = "${filepart}" || break
                done
            done | \
            sort -u | \
            tr '\n' ' '
    )
    cd ..
}

do_action() {
    for action do
        case "${action}" in

            help|\
mkdirs|rmdirs|\
clean_build|clean_dist|clean_tar|\
build_ob|build_lib|build_bin|build_test|build_doc|build_conf|\
dist_ob|dist_lib|dist_bin|dist_test|dist_doc|dist_conf|dist_include|dist_license|\
tar)
                eval "do_${action}"
                printf '[%s] done\n' "${action}" >&2
                ;;

            clean|fullclean|minimal|standard|devel)
                eval "do_action \${actions_${action}}"
                ;;
            maintainer)
                (
                    EXTRA_CC_ARGS="${EXTRA_CC_ARGS_MAINTAINER}"
                    do_action ${actions_maintainer}
                )
                ;;

            *)
                printf 'Unrecognised command "%s"\n' "$action" >&2
                exit 1
                ;;
        esac
    done
}

script_path="$(realpath -e "${0}")"
script_dir="$(dirname "${script_path}")"
project_name="$(basename "${script_dir}")"
actions_clean='clean_dist clean_build clean_tar'
actions_fullclean='clean rmdirs'
actions_minimal='mkdirs clean build_ob build_lib build_bin build_conf dist_lib dist_bin dist_conf dist_license tar'
actions_standard='mkdirs clean build_ob build_lib build_bin build_doc build_conf dist_lib dist_bin dist_doc dist_conf dist_license tar'
actions_devel='mkdirs clean build_ob build_lib build_bin build_doc build_conf dist_ob dist_lib dist_bin dist_doc dist_conf dist_include dist_license tar'
actions_maintainer='mkdirs clean build_ob build_lib build_bin build_test build_doc build_conf dist_ob dist_lib dist_bin dist_test dist_doc dist_conf dist_include dist_license tar'
actions_default='standard'

cd "${script_dir}"
test ${#} -ne 0 || set -- "${actions_default}"

setup_vars
do_action ${*}
