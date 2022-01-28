
GREEN="\e[32;1m"
RESET="\e[0m"
SCRIPTS=scripts
SHELL=/bin/bash

.PHONY: network
network:
	@$(SCRIPTS)/docker-network.sh create

.PHONY: pre-build
pre-build: network
	@cd images/base; docker build -t ubuntu-minimum:0.1 .
	
.PHONY: build-docker
build-docker: pre-build
	@echo -e $(GREEN)"=== Building docker images"$(RESET)
	@cd images; docker-compose build

.PHONY: build-applications
build-applications:
	@echo -e $(GREEN)"=== Building application"$(RESET)
	@cd application/my-code; make
	@if [ ! -d bin ]; then mkdir bin; fi
	@cp application/my-code/server bin
	@cp application/my-code/client bin

.PHONY: init
init: build-all
	@echo -e $(GREEN)"=== Starting docker-compose containers"$(RESET)
	@docker-compose up -d
	$(SCRIPTS)/init-kdc.sh
	$(SCRIPTS)/copy-binaries.sh

.PHONY: build-all	
build-all: build-docker build-applications
	@cd images; docker-compose build


.PHONY: test
test:
	$(SCRIPTS)/test.sh

clean:
	@echo -e $(GREEN)"=== Cleaning.."$(RESET)
	@docker-compose down
	@$(SCRIPTS)/docker-network.sh destroy
	@cd application/my-code; make clean
	@rm bin/ -rf || echo "hello" > /dev/null
