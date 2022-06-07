#!/bin/bash

SECURE_JIT=""
cOS=$(uname -a | awk '{print $(1)}')
cPLT=$(uname -a | awk '{print $(NF)}')
if [ "$cOS" == "Darwin" ] && [ "$cPLT" == "arm64" ]; then
    SECURE_JIT="--jit-secure"
fi

LARGE_PAGES=""
if [ -f /proc/sys/vm/nr_hugepages ]
then
    if [ $(cat /proc/sys/vm/nr_hugepages) -gt 0 ]
    then
        LARGE_PAGES="--large-pages"
    fi
fi

HARD_AES=""
if [ $(grep aes /proc/cpuinfo | wc -c) -ne 0 ]
then
  HARD_AES="--hard-aes"
fi

if test -f "/usr/local/bin/coreminer"; then
  coreminer --noeval $LARGE_PAGES $HARD_AES $SECURE_JIT $@
else
  echo "Miner not found!"
  exit 2
fi
