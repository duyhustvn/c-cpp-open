#include <cstdlib>
#include <err.h>
#include <iostream>
#include <string>
#include <vector>

class KafkaConfig {
    private:
        std::string brokers;
        std::string consumer_group;
        std::string statistics_interval_ms;
        bool do_config_dump;
        std::string topic;

    public:
        bool load_kafka_config(std::string &errstr) {
            brokers = getenv("KAFKA_BROKERS");
            if (brokers.empty()) {
                errstr = "failed to load kafka brokers config";
                return false;
            }
            consumer_group = getenv("KAFKA_CONSUMER_GROUP");
            if (consumer_group.empty()) {
                errstr = "failed to load kafka consumer group config";
                return false;
            }
            statistics_interval_ms = getenv("KAFKA_STATISTICS_INTERVAL_MS");
            if (statistics_interval_ms.empty()) {
                errstr = "failed to load statistics interval ms config";
                return false;
            }
            std::string dump_config =  getenv("KAKFA_DO_CONFIG_DUMP");
            if (dump_config.empty()) {
                do_config_dump = false;
            } else {
                do_config_dump = dump_config == "true" ? true : false;
            }
            topic = getenv("KAFKA_TOPIC");
            if (topic.empty()) {
                errstr = "failed to load kafka topic config";
                return false;
            }
            return true;
        }

        std::string get_brokers() {
            return brokers;
        }

        std::string get_consumer_group() {
            return consumer_group;
        }

        std::string get_statistics_interval_ms() {
            return statistics_interval_ms;
        }

        bool get_do_config_dump() {
            return do_config_dump;
        }

        std::string get_topic() {
            return topic;
        }
};
