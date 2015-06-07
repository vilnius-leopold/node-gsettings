all:

clean:
	rm -rf build/
	rm -f npm-debug.log

build: clean nodegsettings.cc
	node-gyp configure build

test:
	npm test

build-and-test: build test
