# $@ The name of the target file (the one before the colon)
# $< The name of the first (or only) prerequisite file (the first one after the colon)
# $^ The names of all the prerequisite files (space-separated)
# $* The stem (the bit which matches the `%` wildcard in the rule definition.


export CXXFLAGS=-I$(shell pwd)/vcpkg_installed/x64-linux/include
export CFLAGS=-Wall -I$(shell pwd)/vcpkg_installed/x64-linux/include
export LDFLAGS=-L$(shell pwd)/vcpkg_installed/x64-linux/lib -lz -lrt -lpthread -lrdkafka -lm -llz4
# export PKG_CONFIG_PATH=$(shell pwd)/vcpkg_installed/x64-linux/lib/pkgconfig:$(shell pwd)/installed/x64-linux/share/pkgconfig:$PKG_CONFIG_PATH

ENVFLAGS=-DENV_PRODUCT

.PHONY: build-producer build-consumer clean
build-producer: producer
producer: producer.c
	gcc $(CFLAGS) $< -o $@ $(LDFLAGS) $(ENVFLAGS)

build-consumer: consumer
consumer: consumer.c
	gcc $(CFLAGS) $< -o $@ $(LDFLAGS) $(ENVFLAGS)

clean:
	rm producer consumer
