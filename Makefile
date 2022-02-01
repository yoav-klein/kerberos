
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
	@echo -e $(GREEN)"=== Building krb5-api"$(RESET)
	@cd application/krb5-api; make
	@echo -e $(GREEN)"=== Building GSS-API"$(RESET)
	@cd application/gssapi; make
	@$(SCRIPTS)/create-deployment.sh

.PHONY: start-containers
	@docker-compose up -d

.PHONY: init
init: start-containers
	@echo -e $(GREEN)"=== Starting docker-compose containers"$(RESET)
	@docker-compose up -d
	@$(SCRIPTS)/init-kdc.sh
	@$(SCRIPTS)/copy-binaries.sh

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
	@cd application/krb5-api; make clean
	@cd application/gssapi; make clean
	@rm bin/ -rf || echo "hello" > /dev/null
