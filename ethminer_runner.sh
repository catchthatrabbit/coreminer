LARGE_PAGES=""
if [ $(cat /proc/sys/vm/nr_hugepages) -gt 0 ]
then
  LARGE_PAGES="--large-pages"
fi

HARD_AES=""
if [ $(grep aes /proc/cpuinfo | wc -c) -ne 0 ]
then
  HARD_AES="--hard-aes"
fi

./ethminer --noeval $LARGE_PAGES $HARD_AES $@
