/**
 * PubSub.h
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
#ifndef PUBSUB_H
#define PUBSUB_H

#include <stdint.h>
#include <stdio.h>

typedef struct{
    char * topicName;
    char * projectId;
    char * subscription_id;
}PubSubTopic;

typedef struct{
    char * message;
    _Bool posted_ok;
    _Bool posted_error;
    char * message_id;
}PushMessage;

typedef struct {
    char *data;
    char *messageId;
    char *publishTime;
} Message;

typedef struct{
    Message * message_array;
    _Bool received_ok;
    _Bool received_error;
    int msg_count;
}PullMessage;

typedef struct{
    char *response;
    _Bool transfer_completed;
}httpResponse;

void postMessage(char* access_token, PushMessage *myMsg,PubSubTopic *Topic);
void pullMessages(char* access_token , PullMessage*,PubSubTopic*);
static char *base64_decode(const char *encoded);

#endif // WIFI_MANAGER_H

