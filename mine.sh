#!/bin/bash

units_available()
{
	if punits=$(nproc) ; then
		echo $punits
	else
		echo ""
	fi
}

request_worker_name() {
	echo "$(tput setaf 3)●$(tput sgr 0) Enter worker name. Choose format:"
	echo "1) Regular name (alphanumeric and underscores/hyphens)"
	echo "2) Fediverse user (e.g., _usernameInstanceTld-id)"
	echo "3) Random (e.g., worker-34874)"
	read -p "$(tput setaf 3)➤$(tput sgr 0) Selection (1, 2, or 3): " choice

	case $choice in
		1)
			while true; do
				read -p "$(tput setaf 3)➤$(tput sgr 0) Enter regular name: " worker
				if [[ $worker =~ ^[a-zA-Z0-9_-]+$ ]]; then
					break
				else
					echo "$(tput setaf 1)●$(tput sgr 0) Invalid format, please use alphanumeric characters, underscores, or hyphens."
				fi
			done
			;;
		2)
			read -p "$(tput setaf 3)➤$(tput sgr 0) Enter username (alphanumeric): " username
			while ! [[ $username =~ ^[a-zA-Z0-9]+$ ]]; do
				echo "$(tput setaf 1)●$(tput sgr 0) Invalid username, must be alphanumeric."
				read -p "$(tput setaf 3)➤$(tput sgr 0) Enter username (alphanumeric): " username
			done
			username=$(echo "$username" | awk '{print tolower($0)}') # convert to lowercase

			read -p "$(tput setaf 3)➤$(tput sgr 0) Enter domain (instance.tld): " domain
			# Transform domain according to the specification
			formatted_domain=$(echo "$domain" | sed -E 's/(^|\.)([a-z])/\U&/g' | tr -d '.')

			read -p "$(tput setaf 3)➤$(tput sgr 0) Enter optional ID (alphanumeric, can be skipped): " id
			while ! [[ $id =~ ^[a-zA-Z0-9]*$ ]]; do
				echo "$(tput setaf 1)●$(tput sgr 0) Invalid ID, must be alphanumeric."
				read -p "$(tput setaf 3)➤$(tput sgr 0) Enter optional ID (alphanumeric, can be skipped): " id
			done

			worker="_${username}${formatted_domain}"
			if [[ -n $id ]]; then
				worker+="-${id}"
			fi
			;;
		3)
			random_number=$(shuf -i 10000-99999 -n 1) # Generate a random number
			worker="worker-${random_number}"
			;;
		*)
			echo "$(tput setaf 1)●$(tput sgr 0) Invalid choice, try again."
			request_worker_name
			;;
	esac
	echo "$(tput setaf 2)●$(tput sgr 0) Worker name is: $worker"
}

add_pool()
{
	if [[ "$1" -gt "1" ]]; then
		echo
		echo "$(tput setaf 3)●$(tput sgr 0) Please, select the additional mining pool."
		PS3="➤ Additional Pool: "
	else
		echo "$(tput setaf 3)●$(tput sgr 0) Please, select the mining pool."
		PS3="$(tput setaf 3)➤$(tput sgr 0) Pool: "
	fi

	units=`units_available`
	options=("DACH Pool 🇩🇪🇦🇹🇨🇭" "Northeast EU Pool 🇫🇮" "ASEAN Pool 🇸🇬" "East Asian Pool 🇭🇰" "American Pool 🌎" "American 1 Pool 🌎" "Other" "Exit")
	IFS=$'\n'
	select opt in "${options[@]}"
	do
		case "$REPLY" in
			1)
				pool_heading $opt
				server[$1]="de.catchthatrabbit.com"
				port[$1]=8008
				if [[ "$1" -lt "2" ]]; then
					read -p "$(tput setaf 3)➤$(tput sgr 0) Enter wallet address: " wallet
					request_worker_name
					printf "$(tput setaf 3)●$(tput sgr 0) Available processing units: %s\n" $units
					read -p "$(tput setaf 3)➤$(tput sgr 0) How many units to use? [Enter for all] " threads
				fi
				break
				;;
			2)
			pool_heading $opt
			server[$1]="fi.catchthatrabbit.com"
				port[$1]=8008
				if [[ "$1" -lt "2" ]]; then
					read -p "$(tput setaf 3)➤$(tput sgr 0) Enter wallet address: " wallet
					request_worker_name
					printf "$(tput setaf 3)●$(tput sgr 0) Available processing units: %s\n" $units
					read -p "$(tput setaf 3)➤$(tput sgr 0) How many units to use? [Enter for all] " threads
				fi
				break
				;;
			3)
			pool_heading $opt
			server[$1]="sg.catchthatrabbit.com"
				port[$1]=8008
				if [[ "$1" -lt "2" ]]; then
					read -p "$(tput setaf 3)➤$(tput sgr 0) Enter wallet address: " wallet
					request_worker_name
					printf "$(tput setaf 3)●$(tput sgr 0) Available processing units: %s\n" $units
					read -p "$(tput setaf 3)➤$(tput sgr 0) How many units to use? [Enter for all] " threads
				fi
				break
				;;
			4)
			pool_heading $opt
			server[$1]="hk.catchthatrabbit.com"
				port[$1]=8008
				if [[ "$1" -lt "2" ]]; then
					read -p "$(tput setaf 3)➤$(tput sgr 0) Enter wallet address: " wallet
					request_worker_name
					printf "$(tput setaf 3)●$(tput sgr 0) Available processing units: %s\n" $units
					read -p "$(tput setaf 3)➤$(tput sgr 0) How many units to use? [Enter for all] " threads
				fi
				break
				;;
			5)
			pool_heading $opt
			server[$1]="am.catchthatrabbit.com"
				port[$1]=8008
				if [[ "$1" -lt "2" ]]; then
					read -p "$(tput setaf 3)➤$(tput sgr 0) Enter wallet address: " wallet
					request_worker_name
					printf "$(tput setaf 3)●$(tput sgr 0) Available processing units: %s\n" $units
					read -p "$(tput setaf 3)➤$(tput sgr 0) How many units to use? [Enter for all] " threads
				fi
				break
				;;
			6)
			pool_heading $opt
			server[$1]="am1.catchthatrabbit.com"
				port[$1]=8008
				if [[ "$1" -lt "2" ]]; then
					read -p "$(tput setaf 3)➤$(tput sgr 0) Enter wallet address: " wallet
					request_worker_name
					printf "$(tput setaf 3)●$(tput sgr 0) Available processing units: %s\n" $units
					read -p "$(tput setaf 3)➤$(tput sgr 0) How many units to use? [Enter for all] " threads
				fi
				break
				;;
			7)
			pool_heading $opt
			read -p "$(tput setaf 3)➤$(tput sgr 0) Enter server address: " server[$1]
				read -p "$(tput setaf 3)➤$(tput sgr 0) Enter server port: " port[$1]
				if [[ "$1" -lt "2" ]]; then
					read -p "$(tput setaf 3)➤$(tput sgr 0) Enter wallet address: " wallet
					request_worker_name
					printf "$(tput setaf 3)●$(tput sgr 0) Available processing units: %s\n" $units
					read -p "$(tput setaf 3)➤$(tput sgr 0) How many units to use? [Enter for all] " threads
				fi
				break
				;;
			8) clear; exit 0;;
			*) echo "$(tput setaf 1)●$(tput sgr 0) Invalid option."; continue;;
		esac
	done
	IFS=$' \t\n'
}

pool_heading()
{
	echo
	echo "╒════════════════════════════════════"
	echo "│ $1"
	echo "╘════════════════════════════════════"
}

start_mining()
{

	SECURE_JIT=""
	cOS=$(uname -a | awk '{print $(1)}')
	cPLT=$(uname -m)
	if [ "$cOS" == "Darwin" ] && [ "$cPLT" == "arm64" ]; then
		SECURE_JIT="--jit-secure"
	fi

	LARGE_PAGES=""
	if [ -f /proc/sys/vm/nr_hugepages ]; then
		if [ $(cat /proc/sys/vm/nr_hugepages) -gt 0 ]; then
			LARGE_PAGES="--large-pages"
		fi
	fi

	HARD_AES=""
	if [ $(grep aes /proc/cpuinfo 2>&1 | wc -c) -ne 0 ];	then
		HARD_AES="--hard-aes"
	fi

	POOLS=""
	for pool in "${@:2}"
	do
		POOLS+="-P ${pool} "
	done

	THREAD=""
	if [[ "$1" -gt "0" ]]; then
		THREAD="-t ${1}"
	fi

	if [ ! -f "coreminer" ]; then
		echo "$(tput setaf 1)●$(tput sgr 0) Miner not found!"
		exit 2
	fi

	if [[ -x "coreminer" ]]; then
		./coreminer --noeval $LARGE_PAGES $HARD_AES $SECURE_JIT $POOLS $THREAD
	else
		chmod +x coreminer
		./coreminer --noeval $LARGE_PAGES $HARD_AES $SECURE_JIT $POOLS $THREAD
	fi
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
			echo "$(tput setaf 1)●$(tput sgr 0) Invalid wallet!"
			exit 1
		fi
	else
		echo "$(tput setaf 3)●$(tput sgr 0) Not able to validate wallet! (Install 'bc' if needed.)"
	fi
}

compose_stratum()
{
	# scheme://wallet[.workername][:password]@hostname:port[/...]
	if [[ -z "$4" ]]; then
		echo "stratum1+tcp://$1@$2:$3"
	else
		echo "stratum1+tcp://$1.$4@$2:$3"
	fi
}

export_config()
{
	> $1
	for setting in "${@:2}"
	do
		echo $setting >> $1
	done
}

import_config()
{
	. $1
}

update_app()
{
	if [ -f "./mine.updated.sh" ]; then
		mv -f mine.updated.sh mine.sh
	fi
	JSONDATA=$(curl -X GET --header "Accept: application/json" "https://api.github.com/repos/catchthatrabbit/coreminer/releases/latest")
	TAG=$(echo "${JSONDATA}" | awk 'BEGIN{RS=","} /tag_name/{gsub(/.*: "/,"",$0); gsub(/"/,"",$0); print $0}')
	LATESTVER=$(echo ${TAG} | sed -r 's/^v//')
	ARCH=$(uname -m)
	if [ "$ARCH" == "aarch64" ]; then ARCH="arm64"; fi
	PLATFORM=$(uname | tr '[:upper:]' '[:lower:]')
	LATESTDOWN="https://github.com/catchthatrabbit/coreminer/releases/download/${TAG}/coreminer-${PLATFORM}-${ARCH}.tar.gz"
	if [ -f "./coreminer" ]; then
		VER=$(./coreminer -V | sed -n '2p' | sed 's/+commit\.\?[a-f0-9]*//')
		printf -v versions '%s\n%s' "$VER" "$LATESTVER"
		if [[ $versions = "$(sort -V <<< "$versions")" ]]; then
			if curl --output /dev/null --silent --head --fail "$LATESTDOWN"; then
				echo "$(tput setaf 2)●$(tput sgr 0) Downloading the update."
				curl -OL "$LATESTDOWN"
				tar -xzvf ./"coreminer-${PLATFORM}-${ARCH}.tar.gz"
				rm -f ./"coreminer-${PLATFORM}-${ARCH}.tar.gz"
				cd coreapp && mv -f coreminer ../coreminer && mv -f mine.sh ../mine.updated.sh
				cd .. && rm -rf coreapp
				echo "$(tput setaf 2)●$(tput sgr 0) Restarting the program."
				exec ./mine.sh
			else
				echo "$(tput setaf 3)●$(tput sgr 0) Update not found for your system!"
			fi
		else
			echo "$(tput setaf 2)●$(tput sgr 0) You have the latest version already."
		fi
	else
		echo "$(tput setaf 2)●$(tput sgr 0) Software is not installed in this folder. Downloading the latest version."
		curl -OL "$LATESTDOWN"
		FILENAME="coreminer-${PLATFORM}-${ARCH}.tar.gz"
		GZIP_MAGIC_NUMBER=$(head -c 2 "${FILENAME}" | od -N 2 -t x1 | awk 'NR==1 { printf("%s%s\n", $2, $3) }')
		if [ "${GZIP_MAGIC_NUMBER}" == "1f8b" ]; then
			tar -xzvf ./"coreminer-${PLATFORM}-${ARCH}.tar.gz"
			rm -f ./"coreminer-${PLATFORM}-${ARCH}.tar.gz"
			cd coreapp && mv -f coreminer ../coreminer && mv -f mine.sh ../mine.updated.sh
			cd .. && rm -rf coreapp
			echo "$(tput setaf 2)●$(tput sgr 0) Restarting the program."
			exec ./mine.updated.sh
		else
			echo "$(tput setaf 1)●$(tput sgr 0) Downloaded file is not in gzip format. Update failed."
			echo "$(tput setaf 1)●$(tput sgr 0) Please, download the latest version manually."
			rm -f ./"coreminer-${PLATFORM}-${ARCH}.tar.gz"
			exit 3
		fi
	fi
}

autostart_service()
{
	if [ -f "/etc/systemd/system/coreminer.service" ]; then
		echo "$(tput setaf 2)●$(tput sgr 0) Autostart service already exists."
		return
	fi
	echo "$(tput setaf 2)●$(tput sgr 0) Creating autostart service."
	> /etc/systemd/system/coreminer.service
	echo "[Unit]" >> /etc/systemd/system/coreminer.service
	echo "Description=CoreVerificator" >> /etc/systemd/system/coreminer.service
	echo "After=network.target" >> /etc/systemd/system/coreminer.service
	echo "StartLimitIntervalSec=0" >> /etc/systemd/system/coreminer.service
	echo "" >> /etc/systemd/system/coreminer.service
	echo "[Service]" >> /etc/systemd/system/coreminer.service
	echo "Type=simple" >> /etc/systemd/system/coreminer.service
	echo "WorkingDirectory=$(pwd)" >> /etc/systemd/system/coreminer.service
	echo "ExecStart=/bin/bash $(pwd)/mine.sh" >> /etc/systemd/system/coreminer.service
	echo "Restart=always" >> /etc/systemd/system/coreminer.service
	echo "RestartSec=3" >> /etc/systemd/system/coreminer.service
	echo "TimeoutStartSec=0" >> /etc/systemd/system/coreminer.service
	echo "" >> /etc/systemd/system/coreminer.service
	echo "[Install]" >> /etc/systemd/system/coreminer.service
	echo "WantedBy=multi-user.target" >> /etc/systemd/system/coreminer.service
	systemctl daemon-reload
	systemctl enable coreminer.service
	echo "$(tput setaf 2)●$(tput sgr 0) Autostart service created."
	echo "$(tput setaf 2)●$(tput sgr 0) Vacuuming journal."
	journalctl --rotate && journalctl --vacuum-time=1d
	echo "$(tput setaf 2)●$(tput sgr 0) Starting autostart service and script."
	systemctl start coreminer.service
}

extdrive_check()
{
	EXTCONF=$(lsblk -nlo mountpoint | awk '$0 ~ /^\/media\/[^/]+\/coredrive/ {print}')
	if [ -z "$EXTCONF" ]; then
		echo "pool.cfg"
	else
		echo "${EXTCONF}/pool.cfg"
	fi
}

while :
do

clear
echo " ▄▀▀ ▄▀▄ █▀▄ ██▀   ██▄ ▄▀▀ █▄█ █▄ █"
echo " ▀▄▄ ▀▄▀ █▀▄ █▄▄   █▄█ ▀▄▄ █ █ █ ▀█"
echo

CONFIG=`extdrive_check`
if [ -f "$CONFIG" ]; then
	echo "$(tput setaf 2)●$(tput sgr 0) Mine settings file '$CONFIG' exists."
	echo "$(tput setaf 2)●$(tput sgr 0) Importing settings."
	import_config $CONFIG
	if [ "$update" = true ]; then
		update_app
	fi
	ICANWALLET=${wallet//[[:blank:]]/}
	validate_wallet $ICANWALLET
	echo "$(tput setaf 2)●$(tput sgr 0) Wallet validated."
	STRATUM=""
	echo "$(tput setaf 2)●$(tput sgr 0) Configuring stratum server."
	for i in "${!server[@]}"
	do
		STRATUM+=`compose_stratum "$ICANWALLET" "${server[$i]}" "${port[$i]}" "$worker"`
		STRATUM+=" "
	done
	echo "$(tput setaf 2)●$(tput sgr 0) Starting mining command."
	start_mining "$threads" $STRATUM
else
	echo "$(tput setaf 3)●$(tput sgr 0) Mine settings file '$CONFIG' doesn't exist."
	echo "$(tput setaf 2)●$(tput sgr 0) Proceeding with setup."
	echo
	while true
	do
		read -r -p "$(tput setaf 3)➤$(tput sgr 0) Check for the update? [Yes/No] " upd
		case $upd in
			[yY][eE][sS]|[yY])
				update_app
				break
					;;
			[nN][oO]|[nN])
				echo "$(tput setaf 2)●$(tput sgr 0) Update skipped."
				break
					;;
			*)
				echo "$(tput setaf 1)➤$(tput sgr 0) Invalid input. [Yes/No]"
					;;
		esac
	done
	LOOP=1
	add_pool $LOOP
	ICANWALLET=${wallet//[[:blank:]]/}
	validate_wallet $ICANWALLET
	echo "$(tput setaf 2)●$(tput sgr 0) Wallet validated."

	echo
	(( LOOP++ ))
	while true
	do
		read -r -p "$(tput setaf 3)➤$(tput sgr 0) Do you wish to add additional pool? [Yes/No] " back
		case $back in
			[yY][eE][sS]|[yY])
				add_pool $LOOP
				(( LOOP++ ))
					;;
			[nN][oO]|[nN])
				break
					;;
			*)
				echo "$(tput setaf 1)➤$(tput sgr 0) Invalid input. [Yes/No]"
					;;
		esac
	done

	echo
	echo "$(tput setaf 2)●$(tput sgr 0) Saving the settings."

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
		read -r -p "$(tput setaf 3)➤$(tput sgr 0) Add the autostart service? [Yes/No] " autostart
		case $autostart in
			[yY][eE][sS]|[yY])
				autostart_service
				exit 0
				break
					;;
			[nN][oO]|[nN])
				break
					;;
			*)
				echo "$(tput setaf 1)➤$(tput sgr 0) Invalid input. [Yes/No]"
					;;
		esac
	done

	echo
	while true
	do
		read -r -p "$(tput setaf 3)➤$(tput sgr 0) Start mining now? [Yes/No] " mine
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
				echo "$(tput setaf 1)➤$(tput sgr 0) Invalid input. [Yes/No]"
					;;
		esac
	done
fi
sleep 60
done
