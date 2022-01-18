
GREEN="\e[32;1m"
RESET="\e[0m"

SHELL=/bin/bash



.PHONY: network
network:
	@./create-network.sh

.PHONY: pre-build
pre-build: network
	@cd images/base; docker build -t ubuntu-minimum:0.1 .
	
.PHONY: build-docker
build-docker: pre-build
	@echo -e $(GREEN)"*** Building docker images"$(RESET)
	@cd images; docker-compose build

.PHONY: build-applications
build-applications:
	@echo -e $(GREEN)"*** Building application"$(RESET)
	@cd application; make
	@if [ ! -d bin ]; then mkdir bin; fi
	@cp application/server/server bin
	@cp application/client/client bin

.PHONY: init
init: build-all
	@echo -e $(GREEN)"*** Copying applications into containers"$(RESET)
	scripts/init.sh
	
build-all: build-docker build-applications
	@cd images; docker-compose up -d --build

clean:
	@echo -e $(GREEN)"*** Cleaning.."$(RESET)
	@docker-compose down
	@cd application; make clean
	@rm bin/ -rf || echo "hello" > /dev/null
