#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"
#include "nvs_flash.h"
#include "wifi_manager.h"  
#include "jwt_manager.h"
#include "PubSub.h"
#include <stdio.h>

#define CLIENT_EMAIL "YOUR CLIENT EMAIL";   

const char PRIVATE_KEY[] = "-----BEGIN PRIVATE KEY-----\n"
                                  "YOUR PRIVATE KEY"
                                  "-----END PRIVATE KEY-----\n";

const char* projectId = "PROJECT ID";
const char* topicName = "Example-Topic";
const char* subscription_id = "Example-Topic-sub";
static const char *TAG = "main";
JWTConfig *myConfig;

void app_main(void) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    wifi_init_sta();

    ESP_LOGI(TAG,"wifi connect status :%s" ,is_Wifi_Connected() ? "Connected":"Disconnected");

    myConfig = new_JWTConfig();

    if(myConfig == NULL){
        ESP_LOGE(TAG, "Failed to allocate memory for JWTConfig");
    }else{
        myConfig->init_JWT_Auth(myConfig);
        myConfig->client_email = CLIENT_EMAIL;
        myConfig->hashSize = 32;
        myConfig->signatureSize = 256;
        myConfig->private_key = PRIVATE_KEY;

        while(myConfig->step != step_valid_token_generated){
            switch (myConfig->step)
            {
                case step_jwt_encoded_genrate_header:
                    jwt_encoded_genrate_header(myConfig);  
                break;
                case step_jwt_encoded_genrate_payload:
                    jwt_encoded_genrate_payload(myConfig);
                break;
                case step_jwt_gen_hash:
                    jwt_gen_hash(myConfig);  
                break;
                case step_sign_jwt:
                    sign_jwt(myConfig);  
                break;
                case step_exchangeJwtForAccessToken:
                    exchangeJwtForAccessToken(myConfig); 
                break;
                case step_valid_token_generated:
                    ;
                break;
            }
        }      
    }
    if(step_valid_token_generated){
        PullMessage myPullMsg;
        PubSubTopic myTopic;
        PushMessage myPushMsg;

        myTopic.projectId = projectId;
        myTopic.topicName = topicName;
        myTopic.subscription_id = subscription_id;
        
        myPushMsg.message = "This is a test message";
        postMessage(myConfig->Access_Token,&myPushMsg,&myTopic);
        pullMessages(myConfig->Access_Token,&myPullMsg,&myTopic);
    }
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000));  
    }
}