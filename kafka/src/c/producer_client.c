#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#ifdef ENV_PRODUCT
#include <librdkafka/rdkafka.h>
#else
#include "../../vcpkg_installed/x64-linux/include/librdkafka/rdkafka.h"
#endif

#include "producer.c"

int main(int argc, char **argv) {
    rd_kafka_t *rk = init_kafka_producer();
    sleep(10);
    char buf[] = "Lorem Ipsum is simply dummy text of the printing and typesetting industry. \
                 Lorem Ipsum has been the industry's standard dummy text ever since the 1500s, \
                 when an unknown printer took a galley of type and scrambled it to make a type specimen book. \
                 It has survived not only five centuries, but also the leap into electronic typesetting, remaining essentially unchanged. \
                 It was popularised in the 1960s with the release of Letraset sheets containing Lorem Ipsum passages, \
                 and more recently with desktop publishing software like Aldus PageMaker including versions of Lorem Ipsum. \
                 Why do we use it? \
                 It is a long established fact that a reader will be distracted by the readable content of a page when looking at its layout. \
                 The point of using Lorem Ipsum is that it has a more-or-less normal distribution of letters, as opposed to using 'Content here, content here',\
                 making it look like readable English. Many desktop publishing packages and web page editors now use Lorem Ipsum as their default model text, \
                 and a search for 'lorem ipsum' will uncover many web sites still in their infancy.\
                 Various versions have evolved over the years, sometimes by accident, sometimes on purpose (injected humour and the like).";

    const char *topic = "sample_topic"; // argument topic to produce to

    // signal handler for clean shutdown
    signal(SIGINT, stop);

    while (run) {
        int res_code = publish_message(rk, buf, topic);
        if (res_code == 1) {
            continue;
        }
        // usleep(1000);
    }

    /* wait for final message to be delivered or fail.
     * rd_kafka_flush() is an abstraction over rd_kafka_poll() which
     * waits for all messages to be delivered
     */
     fprintf(stderr, "%% Flushing final message...\n");
     rd_kafka_flush(rk, 10 * 1000); // wait for max 10 seconds

     /*
     ** If the output queue is still not empty there is an issue with producing messages
     ** to the cluster
      */
     if (rd_kafka_outq_len(rk) > 0) {
         fprintf(stderr, "%% %d message(s) were not delivered\n", rd_kafka_outq_len(rk));
     }

     // destroy producer instance
     rd_kafka_destroy(rk);

     return 0;
}
