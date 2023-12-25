/*
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <stddef.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <ctype.h>

#ifdef ENV_PRODUCT
#include <librdkafka/rdkafka.h>
#else
#include "../../vcpkg_installed/x64-linux/include/librdkafka/rdkafka.h"
#endif

static volatile sig_atomic_t run = 1;

// signal termination of program
static void stop(int sig) {
    run = 0;
}


/*
** return 1 if all bytes are printable, else 0
*/
static int is_printable(const char* buf, size_t size) {
    for (size_t i =0; i < size; i++) {
        if (!isprint((int)buf[i])) {
            return 0;
        }

        return 1;
    }
}

int main(int argc, char **argv) {
    rd_kafka_t *rk; // consumer instance handle
    rd_kafka_conf_t *conf; // temporary configuration object
    rd_kafka_resp_err_t err; // librdkafka API error code
    char errstr[512]; // librdkafka API error reporting buffer

    const char *brokers = "172.17.0.1:9092"; // argument: broker list
    const char *groupid = "test"; // argument: consumer group id

    char *topic = "sample_topic";
    char **topics = &topic; // argument: list of topics to subscribe to

    int topic_cnt = 1; // number of topic to subscribe to
    rd_kafka_topic_partition_list_t *subscription; // subscribed topics


    // configuration
    conf = rd_kafka_conf_new();

    /*
    ** Set bootstrap broker(s) as a comma-separated list of host or host:port
    ** librdkafka will use the bootstrap brokers to acquire the full set of the brokers
    ** from the cluster
     */
    if (rd_kafka_conf_set(conf, "bootstrap.servers", brokers, errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK) {
        fprintf(stderr, "%s\n", errstr);
        rd_kafka_conf_destroy(conf);
        return 1;
    }

    /*
    ** Set the consumer group id
    ** All consumers sharing the same group id will join the same group, and the subscribed topic partitions will be assigned
    ** according to the partition.assignment.strategy
    ** (consumer config prorperty) to the consumers in the group
     */
    if (rd_kafka_conf_set(conf, "group.id", groupid, errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK) {
        fprintf(stderr, "%s\n", errstr);
        rd_kafka_conf_destroy(conf);
        return 1;
    }

    /*
    ** Create consumer instance
    **
    ** NOTE: rd_kafka_new() takes ownership of the conf object
    ** and the application must not reference to it again after this call
     */
     rk = rd_kafka_new(RD_KAFKA_CONSUMER, conf, errstr, sizeof(errstr));
     if (!rk) {
         fprintf(stderr, "%% Failed to create new consumer: %s\n", errstr);
         return 1;
     }

     conf = NULL; // configuration object is now owned, and freed by the rd_kafka_t instance

     /*
     ** Redirect all messages from per-partition queues to the main queue
     ** so that messages can be consumed with one call from all assigned partition
     **
     ** The alternative is to poll the main queue (for events) and each partition queue separately
     ** which requires setting up a rebalance callback and keeping track of the assignment
     ** but that is more complex and typically not recommend
      */
      rd_kafka_poll_set_consumer(rk);

      /* Convert the list of topics to a format suitable for librdkafka */
      subscription = rd_kafka_topic_partition_list_new(topic_cnt);
      for (int i = 0; i < topic_cnt; i++) {
          rd_kafka_topic_partition_list_add(subscription, topics[i], /*The parition is ignored by subscribe()*/ RD_KAFKA_PARTITION_UA);
      }

      // subscribe to the list of topics
      err = rd_kafka_subscribe(rk, subscription);
      if (err) {
          fprintf(stderr, "%% Failed to subscribe to %d topics: %s\n", subscription->cnt, rd_kafka_err2str(err));
          rd_kafka_topic_partition_list_destroy(subscription);
          rd_kafka_destroy(rk);
          return 1;
      }

      fprintf(stderr, "%% Subscribed to %d topic(s), "
              "waiting for rebalance and message ... \n",
              subscription->cnt);

      rd_kafka_topic_partition_list_destroy(subscription);

      // signal handler for clean shutdown
      signal(SIGINT, stop);

      /*
      ** Subscribing to topics will trigger a group rebalance which may take some time to finish
      ** but there is no need for the application to handle this idle period in a special way
      ** since a rebalance may happen at any time
      ** start pooling for message
      */
      while (run) {
          rd_kafka_message_t *rkm;

          rkm = rd_kafka_consumer_poll(rk, 100);
          if (!rkm) {
              continue; /* Timeout: no message within 100ms, try again
                         * This short timeout allows checking for run at frequent intervals
                         */
          }

          /* consumer_poll() will return either a proper message or a consumer error (rkm->err is set)  */
          if (rkm->err) {
              /*
              ** consumer errors are generally to be considered informational as the consumer will automatically
              ** try to recover from all types of errors
               */
               fprintf(stderr, "%% Consumer error: %s\n", rd_kafka_message_errstr(rkm));
               rd_kafka_message_destroy(rkm);
               continue;
          }

          // proper message
          printf("Message on %s [%" PRId32 "] at the offset %" PRId64
                 "(leader epoch %" PRId32 "): \n" ,
                 rd_kafka_topic_name(rkm->rkt),
                 rkm->partition,
                 rkm->offset,
                 rd_kafka_message_leader_epoch(rkm));

          // print the message key
          if (rkm->key && is_printable(rkm->key, rkm->key_len)) {
              printf("key: %.*s\n", (int)rkm->key_len, (const char*) rkm->key);
          } else if (rkm->key) {
              printf("key: (%d bytes)\n", (int) rkm->len, (const char*) rkm->payload);
          }

          // printf message value/payload
          if (rkm->payload && is_printable(rkm->payload, rkm->len)) {
              printf("value: %.*s\n", (int)rkm->len, (const char *)rkm->payload);
          } else if (rkm->payload) {
              printf("Value: (%d bytes)\n", (int) rkm->len);
          }

          rd_kafka_message_destroy(rkm);
      }

      // close the consumer: commit final offsets and leave the group
      fprintf(stderr, "%% Closing consumer\n");
      rd_kafka_consumer_close(rk);

      /* Destroy the consumer */
      rd_kafka_destroy(rk);

      return 0;
}
