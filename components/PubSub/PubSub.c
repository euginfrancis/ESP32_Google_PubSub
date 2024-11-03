/**
 * PubSub.c
 *
 * Created on: 01.11.2024
 *
 * Copyright (c) 2024 Eugin Francis. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */
#include "PubSub.h"
#include <stdio.h>
#include <string.h>
#include "esp_http_client.h"
#include "cJSON.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_log.h"
#include <esp_crt_bundle.h>
#include "jwt_manager.h"
#include "mbedtls/base64.h"

static const char *TAG = "PostPubSub";

static esp_err_t _http_event_handler(esp_http_client_event_t *evt) {
    static int total_len = 0;
    static char *response_data = NULL;
    httpResponse *myResponse = (httpResponse *)evt->user_data;

    switch (evt->event_id) {
       case HTTP_EVENT_ERROR:
            ESP_LOGI(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            if (!esp_http_client_is_chunked_response(evt->client)) {
                response_data = (char *)malloc(evt->data_len + 1);
                if(response_data == NULL){
                    ESP_LOGE(TAG, "Failed to allocate memory for response");
                    return ESP_FAIL; 
                }
                memcpy(response_data, evt->data, evt->data_len);
                response_data[evt->data_len] = 0;
                ESP_LOGI(TAG, "HTTP Response: %s", response_data);
            }else{
                if (response_data == NULL) {
                    response_data = malloc(evt->data_len + 1);
                    if (response_data == NULL) {
                        ESP_LOGE(TAG, "Failed to allocate memory for response");
                        return ESP_FAIL;
                    }
                    memcpy(response_data, evt->data, evt->data_len);
                } else {
                    char *temp = realloc(response_data, total_len + evt->data_len + 1);
                    if (temp == NULL) {
                        ESP_LOGE(TAG, "Failed to allocate memory for response");
                        free(response_data);
                        return ESP_FAIL;
                    }
                    response_data = temp;
                    memcpy(response_data + total_len, evt->data, evt->data_len);
                }
                total_len += evt->data_len;
                response_data[total_len] = 0; 
            }
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_DISCONNETED");

            if(total_len > 0){
                myResponse->response = (char *)malloc(total_len + 1);
                if (myResponse->response == NULL) {
                    ESP_LOGE(TAG, "Failed to allocate memory for response");    
                }else{
                    strcpy(myResponse->response,response_data);
                }
            }
            if(response_data != NULL){
                free(response_data);
            }
            total_len = 0;
            response_data = NULL;
            myResponse->transfer_completed = true;
            break;
        case HTTP_EVENT_HEADERS_SENT: 
            ESP_LOGI(TAG, "HTTP_EVENT_HEADERS_SENT");
            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_FINISH");
        break;
        default: 
            ESP_LOGI(TAG, "Unhandled event: %d", evt->event_id);
            break;
        }
    return ESP_OK;

}
void postMessage(char* access_token,PushMessage *myMsg,PubSubTopic *Topic){
    httpResponse *myResponse = (httpResponse*)calloc(1,sizeof(myResponse));

    esp_http_client_config_t config = {
        .url = (char *)malloc(256),
        .crt_bundle_attach = esp_crt_bundle_attach,
        .event_handler = _http_event_handler,
        .method = HTTP_METHOD_POST,
        .timeout_ms = 10000,
        .buffer_size_tx = 2048,
        .user_data = myResponse,
    };

    snprintf(config.url, 256, "https://pubsub.googleapis.com/v1/projects/%s/topics/%s:publish", Topic->projectId, Topic->topicName);

    esp_http_client_handle_t client = esp_http_client_init(&config);

    char *auth_header = (char *)malloc(sizeof(char));
    *auth_header ='\0';
    concatStrings(&auth_header,"Bearer ");
    concatStrings(&auth_header,access_token);
    //ESP_LOGE(TAG, "auth_header : %s",auth_header);

    esp_http_client_set_header(client, "Authorization", auth_header);
    esp_http_client_set_header(client, "Content-Type", "application/json");

    cJSON *root = cJSON_CreateObject();
    cJSON *messages = cJSON_CreateArray();
    cJSON_AddItemToObject(root, "messages", messages);

    cJSON *message = cJSON_CreateObject();
    cJSON_AddItemToArray(messages, message);

    char *encodeMsg = base64encodeData((unsigned char *)myMsg->message,strlen(myMsg->message));
    cJSON_AddStringToObject(message, "data", encodeMsg);

    cJSON *attributes = cJSON_CreateObject();
    cJSON_AddStringToObject(attributes, "key", "value");
    cJSON_AddItemToObject(message, "attributes", attributes);

    char *jsonString = cJSON_Print(root);
    //ESP_LOGI(TAG, "Json string : %s", jsonString);
    esp_http_client_set_post_field(client, jsonString, strlen(jsonString));
    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK) {
        ;
    } else {
        ESP_LOGE(TAG, "HTTP POST request failed: %s", esp_err_to_name(err));
    }
    esp_http_client_cleanup(client);
    
    while(!myResponse->transfer_completed);

    //ESP_LOGI(TAG,"Response : %s",myResponse->response);

    if (myResponse->response != NULL) {       
        cJSON *json_response = cJSON_Parse(myResponse->response);
        if (json_response == NULL) {
            myMsg->posted_error = true;
            ESP_LOGE(TAG, "Failed to parse JSON response");
        } else {
            cJSON *messageIds = cJSON_GetObjectItem(json_response, "messageIds");
            if (cJSON_IsArray(messageIds)) {
                int count = cJSON_GetArraySize(messageIds);

                if(count){
                    cJSON *messageId = cJSON_GetArrayItem(messageIds, 0);
                    char *msg = messageId->valuestring;
                    myMsg->message_id = (char *)malloc(strlen(msg)+1);
                    if(myMsg->message_id ==  NULL){
                        ESP_LOGE(TAG, "Failed to allocate memory for response");
                        free(myResponse->response);
                        cJSON_Delete(json_response);
                        goto error;
                    }
                    strcpy(myMsg->message_id,msg);
                    myMsg->posted_ok = true;
                }
            }
            cJSON_Delete(json_response);
        }
        free(myResponse->response);
    }else {
        myMsg->posted_error = true;
    }

    error:

    myResponse->response = NULL;
    myResponse->transfer_completed = false;

    if(myMsg->posted_ok){
        ESP_LOGI(TAG, "Posted Message id: %s", myMsg->message_id);
    }

    cJSON_Delete(root);
    free(jsonString);
    free(config.url);
}

void pullMessages(char* access_token, PullMessage *myMsg, PubSubTopic *Topic){
    httpResponse *myResponse = (httpResponse*)calloc(1,sizeof(myResponse));

    esp_http_client_config_t config = {
        .url = (char *)malloc(256),
        .crt_bundle_attach = esp_crt_bundle_attach,
        .event_handler = _http_event_handler,
        .method = HTTP_METHOD_POST,
        .timeout_ms = 10000,
        .buffer_size_tx = 2048,
        .user_data = myResponse,
    };

    snprintf(config.url, 256, "https://pubsub.googleapis.com/v1/projects/%s/subscriptions/%s:pull", Topic->projectId, Topic->subscription_id);
    esp_http_client_handle_t client = esp_http_client_init(&config);

    char *auth_header = (char *)malloc(sizeof(char));
    *auth_header ='\0';
    concatStrings(&auth_header,"Bearer ");
    concatStrings(&auth_header,access_token);
    //ESP_LOGE(TAG, "auth_header : %s",auth_header);

    esp_http_client_set_header(client, "Authorization", auth_header);
    esp_http_client_set_header(client, "Content-Type", "application/json");

    const char *payload = "{\"maxMessages\": 10}";
    esp_http_client_set_post_field(client, payload, strlen(payload));

    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        ;
    } else {
        ESP_LOGE(TAG, "HTTP GET failed: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
    while(!myResponse->transfer_completed);

    //ESP_LOGI(TAG,"Response : %s",myResponse->response);

    cJSON *json_response = cJSON_Parse(myResponse->response);
    if(json_response != NULL){
        cJSON *receivedMessages = cJSON_GetObjectItem(json_response, "receivedMessages");
        if(receivedMessages != NULL){
            int count = cJSON_GetArraySize(receivedMessages);
            myMsg->msg_count = count;
            ESP_LOGI(TAG,"Count : %d",count);
            Message *messages = (Message *)malloc(count * sizeof(Message));
            if(messages != NULL){
                for (int i = 0; i < count; i++) {
                    cJSON *item = cJSON_GetArrayItem(receivedMessages, i);
                    cJSON *message = cJSON_GetObjectItem(item, "message");

                    char *encoded_data = cJSON_GetObjectItem(message, "data")->valuestring;
                    char *messageId = cJSON_GetObjectItem(message, "messageId")->valuestring;
                    char *publishTime = cJSON_GetObjectItem(message, "publishTime")->valuestring;

                    messages[i].data = base64_decode(encoded_data);
                    messages[i].messageId = strdup(messageId);
                    messages[i].publishTime = strdup(publishTime);

                    //ESP_LOGI(TAG,"data :%s , messageId:%s , PublishTime:%s" , messages[i].data,messages[i].messageId,messages[i].publishTime);
                }
                myMsg->message_array = messages;
            }
        }
        cJSON_Delete(json_response);
    }
    if(myResponse->response != NULL){
        free(myResponse->response);
    }
    myResponse->response = NULL;
    myResponse->transfer_completed = false;
    free(config.url);
}
static char *base64_decode(const char *encoded) {
    size_t encoded_len = strlen(encoded);
    size_t output_len = 0;
    size_t decoded_buf_size = (encoded_len * 3) / 4;

    char *decoded_data = (char *)malloc(decoded_buf_size + 1);  // +1 for null terminator
    if (decoded_data == NULL) {
        printf("Memory allocation failed\n");
        return NULL;
    }

    int ret = mbedtls_base64_decode((unsigned char *)decoded_data, decoded_buf_size, &output_len, 
                                    (const unsigned char *)encoded, encoded_len);
    if (ret != 0) {
        printf("Base64 decode failed with error code: %d\n", ret);
        free(decoded_data);
        return NULL;
    }
    
    decoded_data[output_len] = '\0';
    return decoded_data;
}