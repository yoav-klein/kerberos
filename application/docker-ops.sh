
run() {
	docker run -itd --name cont1 --rm  net-utils:0.3
	docker run -itd --name cont2 --rm  net-utils:0.3
}

run_network() {
	docker run -itd --network=mynet --hostname=yoav1 --domainname=local-yoav.com --rm --name=cont1_net net-utils:0.3 
	docker run -itd --network=mynet --hostname=yoav2 --domainname=local-yoav.com --rm --name=cont2_net net-utils:0.3 
}

stop_all() {
	docker container stop $(docker container ls -q)
	docker container prune -f
}
