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

static const char *TAG = "PostPubSub";

static esp_err_t _http_event_handler(esp_http_client_event_t *evt) {
    static int total_len = 0;
    static char *response_data = NULL;
    
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
                char *response = (char *)malloc(evt->data_len + 1);
                if(response == NULL){
                    ESP_LOGE(TAG, "Failed to allocate memory for response");
                    free(response);
                    return ESP_FAIL; 
                }
                memcpy(response, evt->data, evt->data_len);
                response[evt->data_len] = 0;
                ESP_LOGI(TAG, "HTTP Response: %s", response);
                free(response);
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
                ESP_LOGI(TAG, "Total responce length : %d , Response Data : %s",total_len,response_data);
            }
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_DISCONNETED");
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
void postMessage(char* access_token, PubSubMessage *myMsg){

    esp_http_client_config_t config = {
        .url = (char *)malloc(256),
        .crt_bundle_attach = esp_crt_bundle_attach,
        .event_handler = _http_event_handler,
        .method = HTTP_METHOD_POST,
        .timeout_ms = 10000,
        .buffer_size_tx = 2048,
    };

    snprintf(config.url, 256, "https://pubsub.googleapis.com/v1/projects/%s/topics/%s:publish", myMsg->projectId, myMsg->topicName);

    esp_http_client_handle_t client = esp_http_client_init(&config);

    char *auth_header = (char *)malloc(sizeof(char));
    *auth_header ='\0';
    concatStrings(&auth_header,"Bearer ");
    concatStrings(&auth_header,access_token);
    ESP_LOGE(TAG, "auth_header : %s",auth_header);

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

    cJSON_Delete(root);
    free(jsonString);
    esp_http_client_cleanup(client);
    free(config.url);
}