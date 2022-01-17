
SHELL=/bin/bash



.PHONY: network
network:
	@./create-network.sh


build: network
	@cd images; docker-compose up -d --build
