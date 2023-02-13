#!/bin/bash

# Input parameters
# $1 = wallet (required; will be verified)
# $2 = worker name (optional; "" - explicit null - for skip)
# $3 = api ip:port (optional; "" - explicit null - for skip)
# $4 = pool:port (required; one or more)

# Exit codes
# 1 = Wallet not defined.
# 2 = Invalid wallet.
# 3 = No pool defined.
# 4 = Miner is not installed.

start_mining()
{
    SECURE_JIT=""
    cOS=$(uname -a | awk '{print $(1)}')
    cPLT=$(uname -m)
    if [ "$cOS" == "Darwin" ] && ([ "$cPLT" == "arm64" ] || [ "$cPLT" == "aarch64" ]); then
        SECURE_JIT="--jit-secure"
    fi

    LARGE_PAGES=""
    if [ -f /proc/sys/vm/nr_hugepages ]; then
            if [ $(cat /proc/sys/vm/nr_hugepages) -gt 0 ]; then
                    LARGE_PAGES="--large-pages"
            fi
    fi

    HARD_AES=""
    if [ $(grep aes /proc/cpuinfo 2>&1 | wc -c) -ne 0 ];    then
        HARD_AES="--hard-aes"
    fi

    POOLS=""
    for pool in "${@:1}"
    do
        POOLS+="-P ${pool} "
    done

    coreminer --noeval $LARGE_PAGES $HARD_AES $SECURE_JIT $POOLS $ARGS
}

validate_wallet()
{
    BC=$(which bc)
    if [[ -x "$BC" ]]; then
        ord() {
            LC_CTYPE=C printf '%d' "'$1"
        }
        alphabet_pos() {
            if [ -n "$1" ] && [ "$1" -eq "$1" ] 2>/dev/null; then
                echo $1
            else
                UPPER=$(echo "$1" | tr '[:lower:]' '[:upper:]')
                echo $((`ord $UPPER` - 55))
            fi
        }
        ICAN=$1
        COUNTRY=${ICAN:0:2}
        CHECKSUM=${ICAN:2:2}
        BCAN=${ICAN:4}
        BCCO=`echo $BCAN``echo $COUNTRY`
        SUM=""
        for ((i=0; i<${#BCCO}; i++)); do
            SUM+=`alphabet_pos ${BCCO:$i:1}`
        done
        OPERAND=`echo $SUM``echo $CHECKSUM`
        if [[ `echo "$OPERAND % 97" | $BC` -ne 1 ]]; then
            exit 2
        fi
    fi
}

compose_stratum()
{
    # scheme://wallet[.workername][:password]@hostname:port[/...]
    if [[ -z "$3" ]]; then
        echo "stratum1+tcp://$1@$2"
    else
        echo "stratum1+tcp://$1.$3@$2"
    fi
}

ARGS=""
while [ $# -gt 0 ]; do
    case "$1" in
        --wallet)
            WALLET="$2"
            ;;
        --worker)
            if [[ "$2" =~ ^[-0-9a-zA-Z_]{1,50}$ ]]; then
                WORKER="$2"
            fi
            ;;
        --pool)
            POOL+=("$2")
            ;;
        --args)
            ARGS+="$2 "
            ;;
        *)
            printf "* Error: Invalid argument.*\n"
            ;;
    esac
    shift
    shift
done

if [[ -z "$WALLET" ]]; then
    exit 1
else
    ICANWALLET=${WALLET//[[:blank:]]/}
fi

validate_wallet $ICANWALLET

if [[ -z "$WORKER" ]]; then
    RAND=$(( ((RANDOM<<15)|RANDOM) % 63001 + 2000 ))
    WORKER="worker-$RAND"
fi

STRATUM=""
for poolport in "${POOL[@]}"
do
    STRATUM+=`compose_stratum "$ICANWALLET" "$poolport" "$WORKER"`
    STRATUM+=" "
done

printf STRATUM
if [[ -z "$STRATUM" ]]; then
    exit 3
fi

start_mining $STRATUM
