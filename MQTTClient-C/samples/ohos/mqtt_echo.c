/*
 * Copyright (c) 2020, HiHope Community.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "mqtt_echo.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "MQTTClient.h"

#ifdef CMSIS
#include "cmsis_os2.h"
#define LOGI(fmt, ...) printf("[%p] " fmt "\n", osThreadGetId(), ##__VA_ARGS__)
#else
#define _GNU_SOURCE // for syscall
#include <unistd.h>
#include <sys/syscall.h>
static long gettid(void)
{
    return syscall(SYS_gettid);
}
#define LOGI(fmt, ...) printf("[%ld] " fmt "\n", gettid(), ##__VA_ARGS__)
#endif

#define PUB_MSG_NUM 10

static MQTTClient client = {0};
static Network network = {0};
static unsigned char sendbuf[80], readbuf[80];

#if (defined MQTT_TASK)
static void HandleMessage(void* arg)
{
    MQTTClient* c = (MQTTClient*) arg;
    while (c) {
        MutexLock(&c->mutex);
        if (!c->running) {
            LOGI("MQTT background thread exit!");
            break;
        }
        MutexUnlock(&c->mutex);

        MutexLock(&c->mutex);
        if (c->isconnected) {
            LOGI("recving...");
            MQTTYield(c, 10);
        }
        MutexUnlock(&c->mutex);

        // LOGI("waiting...");
        Sleep(10);
    }
}
#endif

// in Hi3861 SDK, we can not create task in AT command execution context.
void MqttEchoInit(void)
{
    NetworkInit(&network);
    MQTTClientInit(&client, &network, 300, sendbuf, sizeof(sendbuf), readbuf, sizeof(readbuf));

#if (defined MQTT_TASK)
    ThreadStart(&client.thread, HandleMessage, &client);
#endif
}

void MqttEchoDeinit(void)
{
#if (defined MQTT_TASK)
    client.running = 0;
    ThreadJoin(&client.thread);
#endif
}

int MqttEchoConnect(const char* host, unsigned short port,
    const char* clientId, const char* username, const char* password)
{
    int rc = 0;
    MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;

    // connect to server with TCP socket
    if ((rc = NetworkConnect(&network, (char*) host, port)) != 0) {
        LOGI("Return code from NetworkConnect is %d", rc);
        return -1;
	}

    if (username != NULL && password != NULL) {
        connectData.username.cstring = (char*) username;
        connectData.password.cstring = (char*) password;
    }
    connectData.MQTTVersion = 3;
    connectData.clientID.cstring = (char*) clientId;

    // send MQTT CONNECT packet
    if ((rc = MQTTConnect(&client, &connectData)) != 0) {
        LOGI("Return code from MQTT connect is %d", rc);
        return -1;
    }
    LOGI("MQTT Connected!");
    return 0;
}

static void OnMessageArrived(MessageData* data)
{
    LOGI("Message arrived on topic %.*s: %.*s", data->topicName->lenstring.len, data->topicName->lenstring.data,
        data->message->payloadlen, data->message->payload);
}

int MqttEchoSubscribe(char* topic)
{
    int rc = 0;
    if ((rc = MQTTSubscribe(&client, topic, QOS2, OnMessageArrived)) != 0) {
        LOGI("Return code from MQTT subscribe is %d", rc);
        return -1;
    }
    return 0;
}

int MqttEchoPublish(char* topic, char* payload)
{
    int rc = 0;
    MQTTMessage message;

    message.qos = QOS1;
    message.retained = 0;
    message.payload = payload;
    message.payloadlen = strlen(payload);

    if ((rc = MQTTPublish(&client, topic, &message)) != 0) {
        LOGI("Return code from MQTT publish is %d", rc);
        return -1;
    }
    return 0;
}

int MqttEchoDisconnect(void)
{
    int rc = 0;
    // send MQTT DISCONNECT packet
    if ((rc = MQTTDisconnect(&client)) != 0) {
        LOGI("Return code from MQTT disconnect is %d", rc);
        return -1;
    }

    // disconnect TCP socket with server
    NetworkDisconnect(&network);
    return 0;
}

int MqttEchoTest(char* topic)
{
    int i;
    MqttEchoSubscribe(topic);

    for (i = 1; i <= PUB_MSG_NUM; i++) {
        char payload[30];
        sprintf(payload, "{message_number: %d}", i);
        MqttEchoPublish(topic, payload);
        LOGI("message '%s' published!", payload);
    }

#if !(defined MQTT_TASK)
    MQTTYield(&client, 1000);
#endif
    Sleep(200);
    return 0;
}
