#!/bin/bash
set -euo pipefail

function do_install() {
  install -D "$1" rootfs/"$2"
  printf "Install %s\n" "$2"
}

do_install build/cobblestone run/stone
do_install build/loader/loader.so run/loader.so
for mod in build/builtin/*.mod; do
  filename=$(basename "$mod")
  do_install "$mod" run/builtin/"$filename"
done

cd rootfs
tar pcf - * | pv -s $(du -sb . | awk '{print $1}') | gzip -9 > ../dist.tar.gz