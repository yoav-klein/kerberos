

network_name=myexample.com

docker network ls | awk '{print $2}' | grep $network_name > /dev/null

if [ $? == 0 ]; then
	echo "Network exists"
	exit 0
fi

docker network create $network_name

if [ $? == 0 ]; then
	echo "Network created !"
fi
