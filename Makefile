SCHEMA_DIR=/usr/share/glib-2.0/schemas

all:

clean:
	node-gyp clean
	rm -f npm-debug.log

build: clean nodegsettings.cc
	node-gyp rebuild # clean configure build

test: reset-test-schemas
	npm test

install-test-schemas:
	sudo cp tests/com.github.vilnius-leopold.node-gsettings.testing.gschema.xml $(SCHEMA_DIR)
	sudo glib-compile-schemas $(SCHEMA_DIR)

reset-test-schemas:
	gsettings reset-recursively com.github.vilnius-leopold.node-gsettings.testing


remove-test-schemas:
	sudo rm $(SCHEMA_DIR)/com.github.vilnius-leopold.node-gsettings.testing.gschema.xml
	sudo glib-compile-schemas $(SCHEMA_DIR)

build-and-test: build test


