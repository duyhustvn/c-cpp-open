# $@ The name of the target file (the one before the colon)
# $< The name of the first (or only) prerequisite file (the first one after the colon)
# $^ The names of all the prerequisite files (space-separated)
# $* The stem (the bit which matches the `%` wildcard in the rule definition.

export CXXFLAGS=-Wall -I$(shell pwd)/vcpkg_installed/x64-linux/include
export CFLAGS=-Wall -I$(shell pwd)/vcpkg_installed/x64-linux/include
export LDFLAGS=-L$(shell pwd)/vcpkg_installed/x64-linux/lib -lrdkafka++ -lrdkafka -lm -llz4
# export PKG_CONFIG_PATH=$(shell pwd)/vcpkg_installed/x64-linux/lib/pkgconfig:$(shell pwd)/installed/x64-linux/share/pkgconfig:$PKG_CONFIG_PATH

ENVFLAGS=-DENV_PRODUCT
CC=g++

OBJS=consumer_c.o consumer_cpp.o producer.o producer_client.o

BUILD_DIR=build
BUILD_OBJS=$(addprefix $(BUILD_DIR)/, $(OBJS))

SRC_DIR=src

IMG=cpp-consumer
IMG_TAG=v1

build-consumer: $(BUILD_DIR)/consumer
$(BUILD_DIR)/consumer: $(SRC_DIR)/cpp/consumer.cpp
	mkdir -p $(BUILD_DIR)
	$(CC) $(CXXFLAGS) $< -o $@ $(LDFLAGS)

run-consumer:
	KAFKA_BROKERS=172.17.0.1:9092 KAFKA_CONSUMER_GROUP=local KAFKA_STATISTICS_INTERVAL_MS=60000 KAKFA_DO_CONFIG_DUMP=true KAFKA_TOPIC=sample_topic $(BUILD_DIR)/consumer

docker-build:
	docker build -t $(IMG):$(IMG_TAG) .

clean:
	rm -r build

# build-producer-dll: producer.so
# producer.so: producer.c
# 	gcc -fPIC -shared $(CFLAGS) $< -o $@ $(LDFLAGS) $(ENVFLAGS)
#
# build-producer: producer
# producer: producer_client.c
# 	gcc $(CFLAGS) $< -o $@ -L. -l:producer.so
#
# run-producer:
# 	export LD_LIBRARY_PATH=$pwd:$LD_LIBRARY_PATH && ./producer
#
# build-consumer-c: consumer
# consumer: consumer.c
# 	gcc $(CFLAGS) $< -o $@ $(LDFLAGS) $(ENVFLAGS)
#
# build-consumer-cpp-all:
# 	make build-consumer-cpp-1 && cp consumer-cpp-1 consumer-cpp-2
#
# build-consumer-cpp-1: consumer-cpp-1
# consumer-cpp-1: consumer.cpp
# 	g++ $(CXXFLAGS) $< -o $@ $(LDFLAGS) $(ENVFLAGS)
