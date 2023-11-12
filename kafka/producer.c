#include <stdio.h>
#include <signal.h>
#include <string.h>

#ifdef ENV_PRODUCT
#include <librdkafka/rdkafka.h>
#else
#include "vcpkg_installed/x64-linux/include/librdkafka/rdkafka.h"
#endif

static volatile sig_atomic_t run = 1;

// signal termination of program
static void stop(int sig) {
    run = 0;
    fclose(stdin); // abort fgets()
}


/**
 * @brief Message delivery report callback.
 *
 * This callback is called exactly once per message, indicating if
 * the message was succesfully delivered
 * (rkmessage->err == RD_KAFKA_RESP_ERR_NO_ERROR) or permanently
 * failed delivery (rkmessage->err != RD_KAFKA_RESP_ERR_NO_ERROR).
 *
 * The callback is triggered from rd_kafka_poll() and executes on
 * the application's thread.
 */
static void dr_msg_cb(rd_kafka_t *rk, const rd_kafka_message_t *rkmessage, void *opaque) {
    if (rkmessage->err) {
        fprintf(stderr, "%% Message delivery failed: %s\n", rd_kafka_err2str(rkmessage->err));
    } else {
        fprintf(stderr,
                        "%% Message delivered (%zd bytes, partition %" PRId32 ")\n",
                        rkmessage->len, rkmessage->partition);
        /* The rkmessage is destroyed automatically by librdkafka */
    }
}

rd_kafka_t* init_kafka_producer() {
    rd_kafka_t *rk; // producer instance handle
    rd_kafka_conf_t *conf; // temporary configuration object
    char errstr[512]; // librdkafka API error reporting buffer

    const char *brokers = "172.17.0.1:9092"; // argument broker list

    // create kafka client configuration place-holder
    conf = rd_kafka_conf_new();

    /* Set bootstrap broker(s) as a comma-separated list of
     * host or host:port (default port 9092).
     * librdkafka will use the bootstrap brokers to acquire the full
     * set of brokers from the cluster.
     */
    if (rd_kafka_conf_set(conf, "bootstrap.servers", brokers, errstr,
                          sizeof(errstr)) != RD_KAFKA_CONF_OK) {
            fprintf(stderr, "%s\n", errstr);
            return NULL;
    }

    // set the delivery report callback
    // this callback will be called once per message to inform the application
    // if delivery succeeded or failed
    // See dr_msg_cb() above
    // The callback is only trigger from rd_kafka_pool() and rd_kafka_flush()
    rd_kafka_conf_set_dr_msg_cb(conf, dr_msg_cb);

    // create producer instance
    // NOTE: rd_kafka_new() takes ownership of the conf object
    // and the application must not reference it again after this call
    rk = rd_kafka_new(RD_KAFKA_PRODUCER, conf, errstr, sizeof(errstr));
    if (!rk) {
        fprintf(stderr, "%% Failed to create new producer: %s\n", errstr);
        return NULL;
    }

    return rk;
}

/*
** res code:
** 1: queue is empty
 */
int publish_message(rd_kafka_t* rk, char* buf, const char* topic) {
    rd_kafka_resp_err_t err;
    size_t len = strlen(buf);

    if (len == 0) {
        // empty line: only serve delivery reports
        rd_kafka_poll(rk, 0 /*non blocking*/);
        return 1;
    }

        /*
        ** Send/Produce message
        ** This is an asynchronous call, on success it will only
        ** enqueue message on the internal producer queue
        ** The actual delivery attempts to the broker are handled by background threads
        ** The previously registered delivery report callback
        ** (dr_msg_cb) is used to signal back to the application
        ** when the message has been delivery (or failed)
         */
retry:
     err = rd_kafka_producev(
         /* Producer handle */
         rk,
         /* Topic name */
         RD_KAFKA_V_TOPIC(topic),
         /* Make a copy of the payload */
         RD_KAFKA_V_MSGFLAGS(RD_KAFKA_MSG_F_COPY),
         /* Message value and length */
         RD_KAFKA_V_VALUE(buf, len),
         /*
         ** Per-Message opaque, provided in delivery report callback
         ** as msg_opaque
          */
          RD_KAFKA_V_OPAQUE(NULL),
         /* End sentinel */
         RD_KAFKA_V_END);

     if (err) {
         // failed to enqueue message for producing
         fprintf(stderr, "%%Failed to produce to topic: %s: %s\n", topic, rd_kafka_err2str(err));

         if (err == RD_KAFKA_RESP_ERR__QUEUE_FULL) {
             /* if the internal queue is full, wait for messages to be delivery and retry
              * The internal queue represent both messages to be sent and messages that have been sent or failed
              * awaiting their delivery report callback to be called
              *
              * The internal queue is limited by the configuration property
              * queue.buffering.max.messages and
              * queue.buffering.max.kbytes
              */
             rd_kafka_poll(rk, 1000 /* block for max 1000ms */);
             goto retry;
         }
     } else {
        fprintf(stderr, "%% Enqueued message (%zd bytes)"
                    "for topic %s\n",
                    len, topic);
    }

    rd_kafka_poll(rk, 0 /*non-blocking*/);
    return 0;
}


