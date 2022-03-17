#!/bin/bash

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
  coreminer --noeval $LARGE_PAGES $HARD_AES $@
else
  if test -f "$PWD/coreminer"; then
    chmod +x coreminer
    ./coreminer --noeval $LARGE_PAGES $HARD_AES $@
  fi
fi
