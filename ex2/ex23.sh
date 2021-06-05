#!/bin/bash

#Yael Avioz 207237421

system_case(){
	
	# Loop through arguments and process them
	for arg_system in "$@"
	do
		case $arg_system in
			--name)
      cat os-release | head -n 1 | awk -F '"' '{print $2}'
			shift # Remove --initialize from processing
			;;
			--version)
      cat os-release | head -n 2 | tail -n 1 | awk -F '"' '{print $2}'
			shift # Remove --initialize from processing
			;;
			--pretty_name)
      cat os-release | head -n 5 | tail -n 1 | awk -F '"' '{print $2}'
			shift # Remove --initialize from processing
			;;
			--home_url)
      cat os-release | head -n 7 | tail -n 1 | awk -F '"' '{print $2}'
			shift # Remove --initialize from processing
			;;
			--support_url)
      cat os-release | head -n 8 | tail -n 1 | awk -F '"' '{print $2}'
			shift # Remove --initialize from processing
			;;
		esac
	done	
}

host_case(){

	# Loop through arguments and process them
	for arg_host in "$@"
	do
   		case $arg_host in
			--static_hostname)
			cat hostnamectl | head -n 1 | awk '{print $3}'
			shift # Remove --initialize from processing
			;;
			--icon_name)
      cat hostnamectl | head -n 2 | tail -n 1 | awk '{print $3}'
			shift # Remove --initialize from processing
			;;
			--machine_id)
      cat hostnamectl | head -n 4 | tail -n 1 | awk '{print $3}'
			shift # Remove --initialize from processing
			;;
			--boot_id)
      cat hostnamectl | head -n 5 | tail -n 1 | awk '{print $3}'
			shift # Remove --initialize from processing
			;;
			--virtualization)
      cat hostnamectl | head -n 6 | tail -n 1 | awk '{print $2}'
			shift # Remove --initialize from processing
			;;
			--kernel) 
      cat hostnamectl | head -n 8 | tail -n 1 | awk '{print $2 " " $3}' 
			shift # Remove --initialize from processing
			;;
			--architecture)
      cat hostnamectl | head -n 9 | tail -n 1 | awk '{print $2}'
			shift # Remove --initialize from processing
			;;
		esac
	done
}

user_input=$1

#input validation
if [ "${user_input}" != system ] && [ "${user_input}" != host ];
then
	echo "Invalid input"
	exit 1
fi

#host
if [ "${user_input}" == host ]
then
	#in case there is no flags
	if [ -z $2 ]
	then
		#print all the file
		cat hostnamectl
	else
		#print the file with the requested flags
		host_case "$@"
	fi
fi

#system
if [ "${user_input}" == system ];
then
	#in case there is no flags
	if [ -z $2 ]
	then 
		#print all the file
		cat os-release
	else
		#print the file with the requested flags
		system_case "$@"
	fi
fi