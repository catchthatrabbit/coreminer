#!/bin/bash

add_pool()
{
	if [[ "$1" -gt "1" ]]; then
		echo
		echo "〉Please, select the backup mining pool."
		PS3="➤ Backup Pool: "
	else
		echo "〉Please, select the mining pool."
		PS3="➤ Pool: "
	fi

	options=("CTR - Europe" "CTR - Asia" "Other" "Exit")
	select opt in "${options[@]}"
	do
		case "$REPLY" in
			1)
				echo
				echo "╒══════════════════════════════"
				echo "│ 🐰 pool $opt"
				echo "╘══════════════════════════════"
				printf -v "server_$1" '%s' 'eu.catchthatrabbit.com'
				printf -v "port_$1" '%i' 4444
				if [[ "$1" -lt "2" ]]; then
					read -p "➤ Enter wallet address: " wallet
					read -p "➤ Enter workder name: " worker
				fi
				break
				;;
			2)
				echo
				echo "╒══════════════════════════════"
				echo "│ 🐰 pool $opt"
				echo "╘══════════════════════════════"
				printf -v "server_$1" '%s' 'as.catchthatrabbit.com'
				printf -v "port_$1" '%i' 4444
				if [[ "$1" -lt "2" ]]; then
					read -p "➤ Enter wallet address: " wallet
					read -p "➤ Enter workder name: " worker
				fi
				break
				;;
			3)
				echo
				echo "╒══════════════════════════════"
				echo "│ Custom pool"
				echo "╘══════════════════════════════"
				read -p "➤ Enter server address: " "server_$1"
				read -p "➤ Enter server port: " "port_$1"
				if [[ "$1" -lt "2" ]]; then
					read -p "➤ Enter wallet address: " wallet
					read -p "➤ Enter workder name: " worker
				fi
				break
				;;
			4) clear; exit 0;;
			*) echo "〉Invalid option."; continue;;
		esac
	done
}

start_mining()
{
	LARGE_PAGES=""
	if [ -f /proc/sys/vm/nr_hugepages ]; then
	    if [ $(cat /proc/sys/vm/nr_hugepages) -gt 0 ]; then
	        LARGE_PAGES="--large-pages"
	    fi
	fi

	HARD_AES=""
	if [ $(grep aes /proc/cpuinfo >/dev/null 2>&1 | wc -c) -ne 0 ];	then
	  HARD_AES="--hard-aes"
	fi

	POOLS=""
	for pool in "$@"
	do
	    POOLS+="-P $pool "
	done

	coreminer --noeval $LARGE_PAGES $HARD_AES $POOLS
}

validate_wallet()
{
	BC=$(which bc)
	if [[ -x "$BC" ]]; then
		ord() {
		    LC_CTYPE=C printf '%d' "'$1"
		}
		alphabet_pos() {
		    echo $((`ord $1` - 55))
		}
		ICAN=$1
		COUNTRY=${ICAN:0:2}
		CHECKSUM=${ICAN:2:2}
		BCAN=${ICAN:4}
		COUNTRYSUM=`alphabet_pos ${COUNTRY:0:1}``alphabet_pos ${COUNTRY:1}`
		OPERAND=`echo $BCAN``echo $COUNTRYSUM``echo $CHECKSUM`
		if [[ `echo "$OPERAND % 97" | $BC` -ne 1 ]]; then
			echo "Invalid wallet!"
			exit 1
		fi
	else
		echo "〉Not able to validate wallet! (Install 'bc' if needed.)"
	fi
}

compose_stratum()
{
	# scheme://wallet[.workername][:password]@hostname:port[/...]
	STRATUM="stratum://$1.$2@$3:$4"
}

export_config()
{
	echo "\c" > $1
	for setting in "${@:2}"
	do
	    echo $setting >> $1
	done
}

import_config()
{
	. $1
}

while :
do

clear
echo "  _____             __  ____"
echo " / ___/__  _______ /  |/  (_)__  ___ ____"
echo "/ /__/ _ \/ __/ -_) /|_/ / / _ \/ -_) __/"
echo "\___/\___/_/  \__/_/  /_/_/_//_/\__/_/"
echo

CONFIG=pool.cfg
if [ -f "$CONFIG" ]; then
    echo "〉Mine settings file '$CONFIG' exists."
	echo "〉Importing settings."
	import_config $CONFIG
	ICANWALLET=${wallet//[[:blank:]]/}
	validate_wallet $ICANWALLET
	echo "〉Wallet validated."
	if [[ -z "$server_2" ]]; then
		echo "〉Configuring primary stratum server."
		compose_stratum $ICANWALLET $worker $server_1 $port_1
		STRATUM1=$STRATUM
		echo "〉Starting mining command."
		start_mining $STRATUM1
	else
		echo "〉Configuring primary and backup stratum server."
		compose_stratum $ICANWALLET $worker $server_1 $port_1
		STRATUM1=$STRATUM
		compose_stratum $ICANWALLET $worker $server_2 $port_2
		STRATUM2=$STRATUM
		echo "〉Starting mining command."
		start_mining $STRATUM1 $STRATUM2
	fi
else
    echo "〉Mine settings file '$CONFIG' doesn't exist."
	echo "〉Proceeding with setup."
	echo
	add_pool 1
	ICANWALLET=${wallet//[[:blank:]]/}
	validate_wallet $ICANWALLET
	echo "〉Wallet validated."

	echo
	while true
	do
		read -r -p "➤ Do you wish to add backup pool? [yes/no] " back
		case $back in
			[yY][eE][sS]|[yY])
				add_pool 2
				backpool=1
				break
	            ;;
			[nN][oO]|[nN])
	            backpool=0
				break
	            ;;
			*)
	            echo "Invalid input. [yes,no]"
	            ;;
		esac
	done

	echo
	echo "➤ Saving the settings."

	if [[ "$backpool" -gt "0" ]]; then
		export_config $CONFIG "server_1=$server_1" "port_1=$port_1" "server_2=$server_2" "port_2=$port_2" "wallet=$ICANWALLET" "worker=$worker"
	else
		export_config $CONFIG "server_1=$server_1" "port_1=$port_1" "wallet=$ICANWALLET" "worker=$worker"
	fi

	echo
	while true
	do
		read -r -p "➤ Start mining now? [yes/no] " mine
		case $mine in
			[yY][eE][sS]|[yY])
				if [[ "$backpool" -gt "0" ]]; then
					compose_stratum $ICANWALLET $worker $server_1 $port_1
					STRATUM1=$STRATUM
					compose_stratum $ICANWALLET $worker $server_2 $port_2
					STRATUM2=$STRATUM
					start_mining $STRATUM1 $STRATUM2
				else
					compose_stratum $ICANWALLET $worker $server_1 $port_1
					STRATUM1=$STRATUM
					start_mining $STRATUM1
				fi
				break
	            ;;
			[nN][oO]|[nN])
	            break
	            ;;
			*)
	            echo "Invalid input. [yes,no]"
	            ;;
		esac
	done
fi
sleep 60
done
