#!/bin/bash

#######
#
#	docker-network.sh
#	
#	create/destroy the docker network used for the kerberos cluster.
#
#	usage:
#		./docker-network create
#		./docker-network destory
#
######

network_name=myexample.com


create() {
	if [ "$network_exists" ]; then
		echo "=== Network exists"
		exit 0
	fi
	
	echo -e "\e[32;1m=== Creating network ${network_name}\e[0m"
	docker network create $network_name

	if [ $? == 0 ]; then
		echo "Network created !"
	fi
}

destroy() {
	if ! [ "$network_exists" ]; then
		echo "=== Network doesn't exist"
		exit 0
	fi
	
	echo -e "\e[32;1m=== Deleting network ${network_name}\e[0m"
	docker network rm $network_name
}

network_exists=$(docker network ls -q --filter name=${network_name})

$1
