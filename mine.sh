#!/bin/bash

add_pool()
{
	if [[ "$1" -gt "1" ]]; then
		echo
		echo "〉Please, select the additional mining pool."
		PS3="➤ Additional Pool: "
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
				server[$1]="eu.catchthatrabbit.com"
				port[$1]=8008
				if [[ "$1" -lt "2" ]]; then
					read -p "➤ Enter wallet address: " wallet
					read -p "➤ Enter workder name: " worker
					#read -p "➤ How many threads to use? [Enter for all] " threads
				fi
				break
				;;
			2)
				echo
				echo "╒══════════════════════════════"
				echo "│ 🐰 pool $opt"
				echo "╘══════════════════════════════"
				server[$1]="as.catchthatrabbit.com"
				port[$1]=8008
				if [[ "$1" -lt "2" ]]; then
					read -p "➤ Enter wallet address: " wallet
					read -p "➤ Enter workder name: " worker
					#read -p "➤ How many threads to use? [Enter for all] " threads
				fi
				break
				;;
			3)
				echo
				echo "╒══════════════════════════════"
				echo "│ Custom pool"
				echo "╘══════════════════════════════"
				read -p "➤ Enter server address: " server[$1]
				read -p "➤ Enter server port: " port[$1]
				if [[ "$1" -lt "2" ]]; then
					read -p "➤ Enter wallet address: " wallet
					read -p "➤ Enter workder name: " worker
					#read -p "➤ How many threads to use? [Enter for all] " threads
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
	for pool in "${@:2}"
	do
		POOLS+="-P ${pool} "
	done

	THREAD=""
	if [[ "$1" -gt "0" ]]; then
		THREAD="-t ${threads} "
	fi

	coreminer --noeval $LARGE_PAGES $HARD_AES $POOLS $THREAD
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
	if [[ -z "$4" ]]; then
		echo "stratum://$1@$2:$3"
	else
		echo "stratum://$1.$4@$2:$3"
	fi
}

export_config()
{
	echo "${@:2}"
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
	STRATUM=""
	echo "〉Configuring stratum server."
	for i in "${!server[@]}"
	do
		STRATUM+=`compose_stratum "$ICANWALLET" "${server[$i]}" "${port[$i]}" "$worker"`
		STRATUM+=" "
	done
	echo "〉Starting mining command."
	start_mining "$threads" $STRATUM
else
    echo "〉Mine settings file '$CONFIG' doesn't exist."
	echo "〉Proceeding with setup."
	echo
	LOOP=1
	add_pool $LOOP
	ICANWALLET=${wallet//[[:blank:]]/}
	validate_wallet $ICANWALLET
	echo "〉Wallet validated."

	echo
	(( LOOP++ ))
	while true
	do
		read -r -p "➤ Do you wish to add additional pool? [yes/no] " back
		case $back in
			[yY][eE][sS]|[yY])
				add_pool $LOOP
				(( LOOP++ ))
	            ;;
			[nN][oO]|[nN])
				break
	            ;;
			*)
	            echo "Invalid input. [yes,no]"
	            ;;
		esac
	done

	echo
	echo "➤ Saving the settings."

	EXPORTDATA=""
	if [[ "$threads" -gt "0" ]]; then
		EXPORTDATA+="$CONFIG wallet=${ICANWALLET} worker=${worker} threads=${threads}"
	else
		EXPORTDATA+="$CONFIG wallet=${ICANWALLET} worker=${worker}"
	fi
	for ((i = 1; i < $LOOP; i++)); do
		EXPORTDATA+=" server[$i]=${server[$i]} port[$i]=${port[$i]}"
	done
	export_config $EXPORTDATA

	echo
	while true
	do
		read -r -p "➤ Start mining now? [yes/no] " mine
		case $mine in
			[yY][eE][sS]|[yY])
				STRATUM=""
				for ((i = 1; i < $LOOP; i++)); do
					STRATUM+=`compose_stratum "$ICANWALLET" "${server[$i]}" "${port[$i]}" "$worker"`
					STRATUM+=" "
				done
				start_mining "$threads" $STRATUM
				break
	            ;;
			[nN][oO]|[nN])
	            exit 0
	            ;;
			*)
	            echo "Invalid input. [yes,no]"
	            ;;
		esac
	done
fi
sleep 60
done
