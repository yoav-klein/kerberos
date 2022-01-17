
SHELL=/bin/bash



.PHONY: network
network:
	@./create-network.sh

pre-build: network
	@cd images/base; docker build -t ubuntu-minimum:0.1 .
	
build: pre-build
	@cd images; docker-compose up -d --build
