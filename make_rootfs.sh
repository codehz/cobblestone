#!/bin/bash
set -euo pipefail
rm -rf rootfs
mkdir -p rootfs/{tmp,proc,dev,sys,var,run/{game,data,builtin},etc}
cd rootfs
function gen() {
  ldd "$1" | tail -n +2 | awk '{ print $3 }' | sed 's/\n//' | sed -E 's/\/usr\/lib(64|32)\/ld-linux/\/lib\1\/ld-linux/'
}
list=($(gen ../build/cobblestone))
list+=($(gen ../test/game/bedrock_server))
list+=(/usr/lib/lib{resolv,nss_{dns,files,myhostname,mymachines}}.so.2)
uniq=($(printf "%s\n" "${list[@]//*libCrypto.so}" | sort -u | tr '\n' ' '))
for entry in ${uniq[@]}; do
    install -D "$entry" "./$entry"
    echo install "$entry"
done