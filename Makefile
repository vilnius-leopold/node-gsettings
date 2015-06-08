all:

clean:
	node-gyp clean
	rm -f npm-debug.log

build: clean nodegsettings.cc
	node-gyp rebuild # clean configure build

test:
	npm test

build-and-test: build test


