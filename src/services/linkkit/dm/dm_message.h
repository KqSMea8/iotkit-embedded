#ifndef _DM_MESSAGE_H_
#define _DM_MESSAGE_H_

#include "iotx_dm_internal.h"

#define DM_MSG_KEY_ID                   "id"
#define DM_MSG_KEY_VERSION              "version"
#define DM_MSG_KEY_METHOD               "method"
#define DM_MSG_KEY_PARAMS               "params"
#define DM_MSG_KEY_CODE                 "code"
#define DM_MSG_KEY_DATA                 "data"

#define DM_MSG_VERSION                  "1.0"

#define DM_MSG_KEY_PRODUCT_KEY          "productKey"
#define DM_MSG_KEY_DEVICE_NAME          "deviceName"
#define DM_MSG_KEY_DEVICE_SECRET        "deviceSecret"
#define DM_MSG_KEY_TIME                 "time"

#define DM_MSG_SIGN_METHOD_SHA256       "Sha256"
#define DM_MSG_SIGN_METHOD_HMACMD5      "hmacMd5"
#define DM_MSG_SIGN_METHOD_HMACSHA1     "hmacSha1"
#define DM_MSG_SIGN_METHOD_HMACSHA256   "hmacSha256"

typedef struct {
	lite_cjson_t id;
	lite_cjson_t version;
	lite_cjson_t method;
	lite_cjson_t params;
}dm_msg_request_payload_t;

typedef struct {
	lite_cjson_t id;
	lite_cjson_t code;
	lite_cjson_t data;
}dm_msg_response_payload_t;

typedef struct {
	int msgid;
	int devid;
	const char *service_prefix;
	const char *service_name;
	char product_key[PRODUCT_KEY_MAXLEN];
	char device_name[DEVICE_NAME_MAXLEN];
	char *params;
	char *method;
}dm_msg_request_t;

typedef struct {
	const char *service_prefix;
	const char *service_name;
	char product_key[PRODUCT_KEY_MAXLEN];
	char device_name[DEVICE_NAME_MAXLEN];
	iotx_dm_error_code_t code;
}dm_msg_response_t;

typedef struct {
	int id;
}dm_msg_ctx_t;

int dm_msg_init(void);
int dm_msg_deinit(void);
int dm_msg_get_id(void);
int _dm_msg_send_to_user(iotx_dm_event_types_t type, char *message);
int dm_msg_send_msg_timeout_to_user(int msg_id, int devid, iotx_dm_event_types_t type);
int dm_msg_uri_parse_pkdn(_IN_ char *uri, _IN_ int uri_len, _IN_ int start_deli, _IN_ int end_deli, _OU_ char product_key[PRODUCT_KEY_MAXLEN], _OU_ char device_name[DEVICE_NAME_MAXLEN]);
int dm_msg_request_parse(_IN_ char *payload, _IN_ int payload_len, _OU_ dm_msg_request_payload_t *request);
int dm_msg_response_parse(_IN_ char *payload, _IN_ int payload_len, _OU_ dm_msg_response_payload_t *response);
int dm_msg_request_all(_IN_ dm_msg_request_t *request);
int dm_msg_request_cloud(_IN_ dm_msg_request_t *request);
int dm_msg_request_local(_IN_ dm_msg_request_t *request);
int dm_msg_response_with_data(_IN_ dm_msg_request_payload_t *request, _IN_ dm_msg_response_t *response, _IN_ char *data, _IN_ int data_len);
int dm_msg_response_local_with_data(_IN_ dm_msg_request_payload_t *request, _IN_ dm_msg_response_t *response, _IN_ char *data, _IN_ int data_len, void *user_data);
int dm_msg_response_without_data(_IN_ dm_msg_request_payload_t *request, _IN_ dm_msg_response_t *response);
int dm_msg_response_local_without_data(_IN_ dm_msg_request_payload_t *request, _IN_ dm_msg_response_t *response, void *user_data);
int dm_msg_property_set(int devid,dm_msg_request_payload_t *request);
int dm_msg_property_get(_IN_ int devid,_IN_ dm_msg_request_payload_t *request,_IN_ char **payload, _IN_ int *payload_len);
int dm_msg_topo_add_notify(_IN_ char *payload, _IN_ int payload_len);
int dm_msg_thing_service_request(_IN_ char product_key[PRODUCT_KEY_MAXLEN], _IN_ char device_name[DEVICE_NAME_MAXLEN], char *identifier, int identifier_len, dm_msg_request_payload_t *request);
int dm_msg_thing_disable(_IN_ char product_key[PRODUCT_KEY_MAXLEN], _IN_ char device_name[DEVICE_NAME_MAXLEN]);
int dm_msg_thing_enable(_IN_ char product_key[PRODUCT_KEY_MAXLEN], _IN_ char device_name[DEVICE_NAME_MAXLEN]);
int dm_msg_thing_delete(_IN_ char product_key[PRODUCT_KEY_MAXLEN], _IN_ char device_name[DEVICE_NAME_MAXLEN]);
int dm_msg_thing_model_down_raw(_IN_ char product_key[PRODUCT_KEY_MAXLEN], _IN_ char device_name[DEVICE_NAME_MAXLEN], _IN_ char *payload, _IN_ int payload_len);
int dm_msg_thing_gateway_permit(_IN_ char *payload, _IN_ int payload_len);
int dm_msg_thing_sub_register_reply(dm_msg_response_payload_t *response);
int dm_msg_thing_sub_unregister_reply(dm_msg_response_payload_t *response);
int dm_msg_thing_topo_add_reply(dm_msg_response_payload_t *response);
int dm_msg_thing_topo_delete_reply(dm_msg_response_payload_t *response);
int dm_msg_topo_get_reply(dm_msg_response_payload_t *response);
int dm_msg_thing_list_found_reply(dm_msg_response_payload_t *response);
int dm_msg_thing_event_property_post_reply(dm_msg_response_payload_t *response);
int dm_msg_thing_event_post_reply(_IN_ char *identifier, _IN_ int identifier_len, _IN_ dm_msg_response_payload_t *response);
int dm_msg_thing_deviceinfo_update_reply(dm_msg_response_payload_t *response);
int dm_msg_thing_deviceinfo_delete_reply(dm_msg_response_payload_t *response);
int dm_msg_thing_dsltemplate_get_reply(dm_msg_response_payload_t *response);
int dm_msg_thing_dynamictsl_get_reply(dm_msg_response_payload_t *response);
int dm_msg_combine_login_reply(dm_msg_response_payload_t *response);
int dm_msg_combine_logout_reply(dm_msg_response_payload_t *response);
int dm_msg_thing_model_up_raw_reply(_IN_ char product_key[PRODUCT_KEY_MAXLEN], _IN_ char device_name[DEVICE_NAME_MAXLEN],char *payload, int payload_len);
int dm_msg_ntp_response(char *payload, int payload_len);
int dm_msg_ext_error_reply(dm_msg_response_payload_t *response);
int dm_msg_dev_core_service_dev(char **payload, int *payload_len);
int dm_msg_cloud_connected(void);
int dm_msg_cloud_disconnect(void);
int dm_msg_cloud_reconnect(void);
int dm_msg_found_device(_IN_ char product_key[PRODUCT_KEY_MAXLEN], _IN_ char device_name[DEVICE_NAME_MAXLEN]);
int dm_msg_remove_device(_IN_ char product_key[PRODUCT_KEY_MAXLEN], _IN_ char device_name[DEVICE_NAME_MAXLEN]);
int dm_msg_register_result(_IN_ char *uri,_IN_ int result);
int dm_msg_unregister_result(_IN_ char *uri,_IN_ int result);
int dm_msg_send_result(_IN_ char *uri,_IN_ int result);
int dm_msg_add_service_result(_IN_ char *uri,_IN_ int result);
int dm_msg_remove_service_result(_IN_ char *uri,_IN_ int result);

/* TODO */
int dm_msg_thing_sub_register(_IN_ char product_key[PRODUCT_KEY_MAXLEN], _IN_ char device_name[DEVICE_NAME_MAXLEN], _OU_ dm_msg_request_t *request);
int dm_msg_thing_sub_unregister(_IN_ char product_key[PRODUCT_KEY_MAXLEN], _IN_ char device_name[DEVICE_NAME_MAXLEN], _OU_ dm_msg_request_t *request);
int dm_msg_thing_topo_add(_IN_ char product_key[PRODUCT_KEY_MAXLEN], _IN_ char device_name[DEVICE_NAME_MAXLEN], _IN_ char device_secret[DEVICE_SECRET_MAXLEN], _OU_ dm_msg_request_t *request);
int dm_msg_thing_topo_delete(_IN_ char product_key[PRODUCT_KEY_MAXLEN], _IN_ char device_name[DEVICE_NAME_MAXLEN], _OU_ dm_msg_request_t *request);
int dm_msg_thing_topo_get(_OU_ dm_msg_request_t *request);
int dm_msg_thing_list_found(_IN_ char product_key[PRODUCT_KEY_MAXLEN], _IN_ char device_name[DEVICE_NAME_MAXLEN], _OU_ dm_msg_request_t *request);
int dm_msg_thing_property_post(_IN_ char product_key[PRODUCT_KEY_MAXLEN], _IN_ char device_name[DEVICE_NAME_MAXLEN], _IN_ char *payload, _OU_ dm_msg_request_t *request);
int dm_msg_thing_event_post(_IN_ char product_key[PRODUCT_KEY_MAXLEN], _IN_ char device_name[DEVICE_NAME_MAXLEN], _IN_ char *method, _IN_ char *payload, _OU_ dm_msg_request_t *request);
int dm_msg_thing_deviceinfo_update(_IN_ char product_key[PRODUCT_KEY_MAXLEN], _IN_ char device_name[DEVICE_NAME_MAXLEN], _IN_ char *payload, _OU_ dm_msg_request_t *request);
int dm_msg_thing_deviceinfo_delete(_IN_ char product_key[PRODUCT_KEY_MAXLEN], _IN_ char device_name[DEVICE_NAME_MAXLEN], _IN_ char *payload, _OU_ dm_msg_request_t *request);
int dm_msg_thing_dsltemplate_get(_OU_ dm_msg_request_t *request);
int dm_msg_thing_dynamictsl_get(_OU_ dm_msg_request_t *request);
int dm_msg_combine_login(_IN_ char product_key[PRODUCT_KEY_MAXLEN], _IN_ char device_name[DEVICE_NAME_MAXLEN], _IN_ char device_secret[DEVICE_SECRET_MAXLEN], _OU_ dm_msg_request_t *request);
int dm_msg_combine_logout(_IN_ char product_key[PRODUCT_KEY_MAXLEN], _IN_ char device_name[DEVICE_NAME_MAXLEN], _OU_ dm_msg_request_t *request);
int dm_msg_thing_lan_prefix_get(_OU_ dm_msg_request_t *request);
#endif
