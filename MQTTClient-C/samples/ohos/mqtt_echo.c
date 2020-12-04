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

/* Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "MQTTClient.h"

#define PUB_MSG_NUM 10

static void OnMessageArrived(MessageData* data)
{
    printf("Message arrived on topic %.*s: %.*s\n", data->topicName->lenstring.len, data->topicName->lenstring.data,
        data->message->payloadlen, data->message->payload);
}

MQTTClient client = {0};
Network network = {0};
unsigned char sendbuf[80], readbuf[80];

// in Hi3861 SDK, we can not create task in AT command execution context.
void MqttEchoInit(void)
{
    int rc = 0;
    NetworkInit(&network);
    MQTTClientInit(&client, &network, 30000, sendbuf, sizeof(sendbuf), readbuf, sizeof(readbuf));

    if ((rc = MQTTStartTask(&client)) != 0) {
        printf("Return code from MQTTStartTask is %d\n", rc);
    }
}

void MqttEchoDeinit(void)
{
    int rc = 0;
    if ((rc = MQTTStopTask(&client)) != 0) {
        printf("Return code from MQTTStopTask is %d\n", rc);
    }
}

int MqttEchoTest(const char* host, unsigned short port)
{
    int rc = 0, i = 0;
    MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;

    if ((rc = NetworkConnect(&network, (char*) host, port)) != 0) {
        printf("Return code from NetworkConnect is %d\n", rc);
        return -1;
	}

    connectData.MQTTVersion = 3;
    connectData.clientID.cstring = "OHOS_sample";

    if ((rc = MQTTConnect(&client, &connectData)) != 0) {
        printf("Return code from MQTT connect is %d\n", rc);
    } else {
        printf("MQTT Connected!\n");
    }

    if ((rc = MQTTSubscribe(&client, "OHOS/sample/#", 2, OnMessageArrived)) != 0) {
        printf("Return code from MQTT subscribe is %d\n", rc);
    }

    for (i = 1; i <= PUB_MSG_NUM; i++) {
        MQTTMessage message;
        char payload[30];

        message.qos = 1;
        message.retained = 0;
        message.payload = payload;
        sprintf(payload, "message number %d", i);
        message.payloadlen = strlen(payload);

        if ((rc = MQTTPublish(&client, "OHOS/sample/a", &message)) != 0) {
            printf("Return code from MQTT publish is %d\n", rc);
        }
    }

    if ((rc = MQTTDisconnect(&client)) != 0) {
        printf("Return code from MQTT disconnect is %d\n", rc);
    }

    NetworkDisconnect(&network);
    return 0;
}
