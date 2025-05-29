/*
 * @Author: jiejie
 * @Github: https://github.com/jiejieTop
 * @Date: 2019-12-11 21:53:07
 * @LastEditTime : 2022-06-15 23:03:30
 * @Description: the code belongs to jiejie, please keep the author information and source code according to the license.
 */
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <pthread.h>

#include "mqtt_config.h"
#include "mqtt_log.h"
#include "mqttclient.h"

#include "ca.inc"

// #define TEST_USEING_TLS

#define EMQX_HOST       "emqx.logfun.xyz"
// #define EMQX_HOST       "bxx.lowfly.cc"
#define EMQX_CLIENT_ID  "76284159_57940712_mac_0_1745242482"
#define EMQX_USER_NAME  "76284159_57940712_mac"
#define EMQX_PASSWORD   "43BFBD7B439EB78D18D8E6DF2C88D1D1C02E6E2739CAC30830B9C39C3C4C8E1C"


// 订阅消息
static void topic1_handler(void* client, message_data_t* msg)
{
    (void) client;
    MQTT_LOG_I("-----------------------------------------------------------------------------------");
    MQTT_LOG_I("%s:%d %s()...\ntopic: %s\nmessage:%s", __FILE_NAME__, __LINE__, __FUNCTION__, msg->topic_name, (char*)msg->message->payload);
    MQTT_LOG_I("-----------------------------------------------------------------------------------");
}


// 定期发布
void *mqtt_publish_thread(void *arg)
{
    mqtt_client_t *client = (mqtt_client_t *)arg;

    char buf[100] = { 0 };
    mqtt_message_t msg;
    memset(&msg, 0, sizeof(msg));
    sprintf(buf, "welcome to mqttclient, this is a publish test...");

    MQTT_LOG_I("%s", buf);

    sleep(2);

    mqtt_list_subscribe_topic(client);

    msg.payload = (void *) buf;

    while(1) {
        sprintf(buf, "welcome to mqttclient, this is a publish test, a rand number: %d ...", random_number());

        MQTT_LOG_I("%s", buf);

        msg.qos = 0;
        mqtt_publish(client, "topic1", &msg);

        msg.qos = 1;
        mqtt_publish(client, "topic2", &msg);

        msg.qos = 2;
        mqtt_publish(client, "topic3", &msg);

        sleep(4);
    }
}

int main(void)
{
    int res;
    pthread_t thread1;
    mqtt_client_t *client = NULL;
    char client_id[64];
    char user_name[64];
    char password[128];

    printf("\nwelcome to mqttclient test...\n");

    // random_string(client_id, 10);
    // random_string(user_name, 10);
    // random_string(password, 10);
    strcpy(client_id, EMQX_CLIENT_ID);
    strcpy(user_name, EMQX_USER_NAME);
    strcpy(password, EMQX_PASSWORD);

    mqtt_log_init();

    client = mqtt_lease();

#ifdef TEST_USEING_TLS
    mqtt_set_port(client, "8883");
    mqtt_set_ca(client, (char*)test_ca_get());
#else
    mqtt_set_port(client, "1883");
#endif

    // mqtt_set_host(client, "120.25.213.14");
    // mqtt_set_host(client, "114.132.177.8");
    mqtt_set_host(client, EMQX_HOST);
    mqtt_set_client_id(client, client_id);
    mqtt_set_user_name(client, user_name);
    mqtt_set_password(client, password);
    mqtt_set_clean_session(client, 1);

    mqtt_connect(client);

    mqtt_subscribe(client, "topic1", QOS0, topic1_handler);
    mqtt_subscribe(client, "topic2", QOS1, NULL);
    mqtt_subscribe(client, "topic3", QOS2, NULL);

    MQTT_LOG_I("HOST: %s", EMQX_HOST);
    MQTT_LOG_I("client_id: %s, user_name: %s, password: %s", client_id, user_name, password);

    res = pthread_create(&thread1, NULL, mqtt_publish_thread, client);
    if(res != 0) {
        MQTT_LOG_E("create mqtt publish thread fail");
        exit(res);
    }

    while (1) {
        sleep(100);
    }
}
