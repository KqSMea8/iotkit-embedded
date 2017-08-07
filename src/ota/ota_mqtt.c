
#ifndef __OTA_MQTT_C_H__
#define __OTA_MQTT_C_H__


#include "iot_export_ota.h"
#include "iot_import_ota.h"
#include "ota_internal.h"

//OSC, OTA signal channel

/* Specify the maximum characters of version */
#define OTA_MQTT_TOPIC_LEN   (64)

typedef struct  {
    void *mqtt;
    const char *product_key;
    const char *device_name;
    char topic_upgrade[OTA_MQTT_TOPIC_LEN];
    ota_cb_fpt cb;
    void *context;
}otamqtt_Struct_t, *otamqtt_Struct_pt;


//Generate topic name according to @ota_topic_type, @product_key, @device_name
//and then copy to @buf.
//0, successful; -1, failed
static int otamqtt_GenTopicName(char *buf, size_t buf_len, const char *ota_topic_type, const char *product_key, const char *device_name)
{
    int ret;

    ret = snprintf(buf,
            buf_len,
            "/ota/device/%s/%s/%s",
            ota_topic_type,
            product_key,
            device_name);

    OTA_ASSERT(ret < buf_len, "buffer should always enough");

    if (ret < 0) {
        OTA_LOG_ERROR("snprintf failed");
        return -1;
    }

    return 0;
}

//report progress of OTA
static int otamqtt_Publish(otamqtt_Struct_pt handle, const char *topic_type, const char *msg)
{
    int ret;
    char topic_name[OTA_MQTT_TOPIC_LEN];
    iotx_mqtt_topic_info_t topic_info;

    memset(&topic_info, 0, sizeof(iotx_mqtt_topic_info_t));
    topic_info.qos = IOTX_MQTT_QOS1;
    topic_info.payload = (void *)msg;
    topic_info.payload_len = strlen(msg);

    //inform OTA to topic: "/ota/device/progress/$(product_key)/$(device_name)"
    ret = otamqtt_GenTopicName(topic_name, OTA_MQTT_TOPIC_LEN, topic_type, handle->product_key, handle->device_name);
    if (ret < 0) {
       OTA_LOG_ERROR("generate topic name of info failed");
       return -1;
    }

    ret = IOT_MQTT_Publish(handle->mqtt, topic_name, &topic_info);
    if (ret < 0) {
        OTA_LOG_ERROR("publish failed");
        return IOT_OTAE_MQTT_PUB;
    }

    return 0;
}


//decode JSON string to get firmware information, like firmware version, URL, file size, MD5.
//return NONE
static void otamqtt_UpgrageCb(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
{
    otamqtt_Struct_pt handle = (otamqtt_Struct_pt) pcontext;
    iotx_mqtt_topic_info_pt topic_info = (iotx_mqtt_topic_info_pt)msg->msg;

    OTA_LOG_DEBUG("topic=%.*s", topic_info->topic_len, topic_info->ptopic);
    OTA_LOG_DEBUG("data of topic=%.*s", topic_info->payload_len, (char *)topic_info->payload);

    OTA_ASSERT(IOTX_MQTT_EVENT_PUBLISH_RECVEIVED == msg->event_type, "invalid event type");

    if (NULL != handle->cb) {
        handle->cb(handle->context, topic_info->payload, topic_info->payload_len);
    }
}


void *osc_Init(const char *product_key, const char *device_name, void *ch_signal, ota_cb_fpt cb, void *context)
{
    int ret;
    char topic_name[OTA_MQTT_TOPIC_LEN];
    otamqtt_Struct_pt h_osc = NULL;

    if (NULL == (h_osc = OTA_MALLOC(sizeof(otamqtt_Struct_t)))) {
        OTA_LOG_ERROR("malloc failed");
        return NULL;
    }

    memset(h_osc, 0, sizeof(otamqtt_Struct_t));

    //subscribe the OTA topic: "/ota/device/upgrade/$(product_key)/$(device_name)"
    ret = otamqtt_GenTopicName(h_osc->topic_upgrade, OTA_MQTT_TOPIC_LEN, "upgrade", product_key, device_name);
    if (ret < 0) {
        OTA_LOG_ERROR("generate topic name of upgrade failed");
        goto do_exit;
    }

    ret = IOT_MQTT_Subscribe(ch_signal, h_osc->topic_upgrade, IOTX_MQTT_QOS1, otamqtt_UpgrageCb, h_osc);
    if (ret < 0) {
        OTA_LOG_ERROR("mqtt subscribe failed");
        goto do_exit;
    }

    h_osc->mqtt = ch_signal;
    h_osc->product_key = product_key;
    h_osc->device_name = device_name;
    h_osc->cb = cb;
    h_osc->context = context;

    return h_osc;

do_exit:
    if (NULL != h_osc) {
         OTA_FREE(topic_name);
    }

    return NULL;
}


int osc_Deinit(void *handle)
{
    if (NULL != handle) {
        OTA_FREE(handle);
    }

    return 0;
}

//report progress of OTA
int osc_ReportProgress(void *handle, const char *msg)
{
    return otamqtt_Publish(handle, "progress", msg);
}


//report version of OTA firmware
int osc_ReportVersion(void *handle, const char *msg)
{
    return otamqtt_Publish(handle, "inform", msg);
}

#endif