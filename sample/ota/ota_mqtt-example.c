 /*
  * Copyright (c) 2014-2016 Alibaba Group. All rights reserved.
  * License-Identifier: Apache-2.0
  *
  * Licensed under the Apache License, Version 2.0 (the "License"); you may
  * not use this file except in compliance with the License.
  * You may obtain a copy of the License at
  *
  *     http://www.apache.org/licenses/LICENSE-2.0
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
  * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "iot_import.h"
#include "iot_export.h"

#if defined(MQTT_ID2_AUTH) && defined(TEST_ID2_DAILY)
#define PRODUCT_KEY             "OvNmiEYRDSY"
#define DEVICE_NAME             "sh_online_sample_mqtt"
#define DEVICE_SECRET           "v9mqGzepKEphLhXmAoiaUIR2HZ7XwTky"
#elif defined(TEST_OTA_PRE)
#define PRODUCT_KEY             "6RcIOUafDOm"
#define DEVICE_NAME             "sh_pre_sample_mqtt"
#define DEVICE_SECRET           "R0OTtD46DSalSpGW7SFzFDIA6fksTC2c"
#else 
#define PRODUCT_KEY             "yfTuLfBJTiL"
#define DEVICE_NAME             "TestDeviceForDemo"
#define DEVICE_SECRET           "fSCl9Ns5YPnYN8Ocg0VEel1kXFnRlV6c"
#endif

// These are pre-defined topics
#define TOPIC_UPDATE            "/"PRODUCT_KEY"/"DEVICE_NAME"/update"
#define TOPIC_ERROR             "/"PRODUCT_KEY"/"DEVICE_NAME"/update/error"
#define TOPIC_GET               "/"PRODUCT_KEY"/"DEVICE_NAME"/get"
#define TOPIC_DATA              "/"PRODUCT_KEY"/"DEVICE_NAME"/data"

#define MSG_LEN_MAX             (1024)

#define EXAMPLE_TRACE(fmt, args...)  \
    do { \
        printf("%s|%03d :: ", __func__, __LINE__); \
        printf(fmt, ##args); \
        printf("%s", "\r\n"); \
    } while(0)

static int      user_argc;
static char **  user_argv;

void event_handle(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
{
    uintptr_t packet_id = (uintptr_t)msg->msg;
    iotx_mqtt_topic_info_pt topic_info = (iotx_mqtt_topic_info_pt)msg->msg;

    switch (msg->event_type) {
        case IOTX_MQTT_EVENT_UNDEF:
            EXAMPLE_TRACE("undefined event occur.");
            break;

        case IOTX_MQTT_EVENT_DISCONNECT:
            EXAMPLE_TRACE("MQTT disconnect.");
            break;

        case IOTX_MQTT_EVENT_RECONNECT:
            EXAMPLE_TRACE("MQTT reconnect.");
            break;

        case IOTX_MQTT_EVENT_SUBCRIBE_SUCCESS:
            EXAMPLE_TRACE("subscribe success, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_SUBCRIBE_TIMEOUT:
            EXAMPLE_TRACE("subscribe wait ack timeout, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_SUBCRIBE_NACK:
            EXAMPLE_TRACE("subscribe nack, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_UNSUBCRIBE_SUCCESS:
            EXAMPLE_TRACE("unsubscribe success, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_UNSUBCRIBE_TIMEOUT:
            EXAMPLE_TRACE("unsubscribe timeout, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_UNSUBCRIBE_NACK:
            EXAMPLE_TRACE("unsubscribe nack, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_PUBLISH_SUCCESS:
            EXAMPLE_TRACE("publish success, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_PUBLISH_TIMEOUT:
            EXAMPLE_TRACE("publish timeout, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_PUBLISH_NACK:
            EXAMPLE_TRACE("publish nack, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_PUBLISH_RECVEIVED:
            EXAMPLE_TRACE("topic message arrived but without any related handle: topic=%.*s, topic_msg=%.*s",
                     topic_info->topic_len,
                     topic_info->ptopic,
                     topic_info->payload_len,
                     topic_info->payload);
            break;

        default:
            EXAMPLE_TRACE("Should NOT arrive here.");
            break;
    }
}

static void _demo_message_arrive(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
{
    iotx_mqtt_topic_info_pt ptopic_info = (iotx_mqtt_topic_info_pt) msg->msg;

    // print topic name and topic message
    EXAMPLE_TRACE("----");
    EXAMPLE_TRACE("Topic: '%.*s' (Length: %d)",
             ptopic_info->topic_len,
             ptopic_info->ptopic,
             ptopic_info->topic_len);
    EXAMPLE_TRACE("Payload: '%.*s' (Length: %d)",
             ptopic_info->payload_len,
             ptopic_info->payload,
             ptopic_info->payload_len);
    EXAMPLE_TRACE("----");
}



int mqtt_client(void)
{
#define OTA_BUF_LEN        (5000)

    int rc = 0, cnt = 0, ota_over = 0;
    void *pclient = NULL, *h_ota = NULL;
    iotx_conn_info_pt pconn_info;
    iotx_mqtt_param_t mqtt_params;
    char *msg_buf = NULL, *msg_readbuf = NULL;
    FILE *fp;
    char buf_ota[OTA_BUF_LEN];

    if (NULL == (fp = fopen("ota.bin", "wb+"))) {
        EXAMPLE_TRACE("open file failed");
        goto do_exit;
    }

    
    if (NULL == (msg_buf = (char *)HAL_Malloc(MSG_LEN_MAX))) {
        EXAMPLE_TRACE("not enough memory");
        rc = -1;
        goto do_exit;
    }

    if (NULL == (msg_readbuf = (char *)HAL_Malloc(MSG_LEN_MAX))) {
        EXAMPLE_TRACE("not enough memory");
        rc = -1;
        goto do_exit;
    }

    /* Initialize device info */
    IOT_CreateDeviceInfo();

    if (0 != IOT_SetDeviceInfo(PRODUCT_KEY, DEVICE_NAME, DEVICE_SECRET)) {
        EXAMPLE_TRACE("set device info failed!");
        rc = -1;
        goto do_exit;
    }

    /* Device AUTH */
    if (0 != IOT_SetupConnInfo()) {
        EXAMPLE_TRACE("AUTH request failed!");
        rc = -1;
        goto do_exit;
    }
    pconn_info = IOT_GetConnInfo();

    /* Initialize MQTT parameter */
    memset(&mqtt_params, 0x0, sizeof(mqtt_params));

    mqtt_params.port = pconn_info->port;
    mqtt_params.host = pconn_info->host_name;
    mqtt_params.client_id = pconn_info->client_id;
    mqtt_params.user_name = pconn_info->username;
    mqtt_params.password = pconn_info->password;
    mqtt_params.pub_key = pconn_info->pub_key;

    mqtt_params.request_timeout_ms = 2000;
    mqtt_params.clean_session = 0;
    mqtt_params.keepalive_interval_ms = 60000;
    mqtt_params.pread_buf = msg_readbuf;
    mqtt_params.read_buf_size = MSG_LEN_MAX;
    mqtt_params.pwrite_buf = msg_buf;
    mqtt_params.write_buf_size = MSG_LEN_MAX;

    mqtt_params.handle_event.h_fp = event_handle;
    mqtt_params.handle_event.pcontext = NULL;


    /* Construct a MQTT client with specify parameter */
    pclient = IOT_MQTT_Construct(&mqtt_params);
    if (NULL == pclient) {
        EXAMPLE_TRACE("MQTT construct failed");
        rc = -1;
        goto do_exit;
    }
    h_ota = OTA_Init(PRODUCT_KEY, DEVICE_NAME, pclient);
    if (NULL == h_ota) {
        rc = -1;
        EXAMPLE_TRACE("initialize OTA failed");
        goto do_exit;
    }

    if (0 != OTA_ReportVersion(h_ota, "iotx_ver_1.0.0")) {
        rc = -1;
        EXAMPLE_TRACE("report OTA version failed");
        goto do_exit;
    }

    HAL_SleepMs(1000);

    do {
 
       EXAMPLE_TRACE("wait ota upgrade command....");

       /* handle the MQTT packet received from TCP or SSL connection */
        IOT_MQTT_Yield(pclient, 200);

        if (OTA_IsFetching(h_ota)) {
            uint32_t last_percent = 0, percent = 0;
            char version[128], md5sum[33];
            uint32_t len, size_downloaded, size_file;
            do {

                len = OTA_FetchYield(h_ota, buf_ota, OTA_BUF_LEN, 10000);
                if (len > 0) {
                    if (1 != fwrite(buf_ota, len, 1, fp)) {
                        EXAMPLE_TRACE("write data to file failed");
                        rc = -1;
                        break;
                    }
                }

                /* get OTA information */
                OTA_Ioctl(h_ota, OTA_GET_FETCHED_SIZE, &size_downloaded, 4);
                OTA_Ioctl(h_ota, OTA_GET_FILE_SIZE, &size_file, 4);
                OTA_Ioctl(h_ota, OTA_GET_MD5SUM, md5sum, 33);
                OTA_Ioctl(h_ota, OTA_GET_VERSION, version, 128);

                last_percent = percent;
                percent = (size_downloaded * 100) / size_file;
                if (percent - last_percent > 0) {
                    OTA_ReportProgress(h_ota, percent, NULL);
                    OTA_ReportProgress(h_ota, percent, "hello");
                }
        	IOT_MQTT_Yield(pclient, 100);
            }while(!OTA_IsFetchFinish(h_ota));

                        
            ota_over = 1;
        }
        HAL_SleepMs(2000);
    } while (!ota_over);

    HAL_SleepMs(200);



do_exit:

    if (NULL != h_ota) {
         OTA_Deinit(h_ota);
    }
    
    if(NULL != pclient) {
        IOT_MQTT_Destroy(pclient);
    } 

    if (NULL != msg_buf) {
        HAL_Free(msg_buf);
    }

    if (NULL != msg_readbuf) {
        HAL_Free(msg_readbuf);
    }

    if (NULL != fp) {
        fclose(fp);
    }

    return rc;
}

int main(int argc, char **argv)
{
    IOT_OpenLog("mqtt");
    IOT_SetLogLevel(IOT_LOG_DEBUG);

    user_argc = argc;
    user_argv = argv;

    mqtt_client();

    IOT_DumpMemoryStats(IOT_LOG_DEBUG);
    IOT_CloseLog();

    EXAMPLE_TRACE("out of sample!");
    return 0;
}

