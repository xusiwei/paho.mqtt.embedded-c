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

/* OHOS includes. */
#include "cmsis_os2.h"

/* LWIP includes. */
#include "lwip/sockets.h"

#include "MQTTClient.h"

void messageArrived(MessageData* data)
{
    printf("Message arrived on topic %.*s: %.*s\n", data->topicName->lenstring.len, data->topicName->lenstring.data,
        data->message->payloadlen, data->message->payload);
}

static void MqttEchoTask(void* arg)
{
	(void) arg;
    /* connect to m2m.eclipse.org, subscribe to a topic, send and receive messages regularly every 1 sec */
    MQTTClient client;
    Network network;
    unsigned char sendbuf[80], readbuf[80];
    int rc = 0,
        count = 0;
    MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;

    NetworkInit(&network);
    MQTTClientInit(&client, &network, 30000, sendbuf, sizeof(sendbuf), readbuf, sizeof(readbuf));

    char* address = "iot.eclipse.org";
    if ((rc = NetworkConnect(&network, address, 1883)) != 0) {
        printf("Return code from network connect is %d\n", rc);
	}

#if defined(MQTT_TASK)
    if ((rc = MQTTStartTask(&client)) != 0)
        printf("Return code from start tasks is %d\n", rc);
#endif

    connectData.MQTTVersion = 3;
    connectData.clientID.cstring = "OHOS_sample";

    if ((rc = MQTTConnect(&client, &connectData)) != 0)
        printf("Return code from MQTT connect is %d\n", rc);
    else
        printf("MQTT Connected\n");

    if ((rc = MQTTSubscribe(&client, "OHOS/sample/#", 2, messageArrived)) != 0)
        printf("Return code from MQTT subscribe is %d\n", rc);

    while (++count)
    {
        MQTTMessage message;
        char payload[30];

        message.qos = 1;
        message.retained = 0;
        message.payload = payload;
        sprintf(payload, "message number %d", count);
        message.payloadlen = strlen(payload);

        if ((rc = MQTTPublish(&client, "OHOS/sample/a", &message)) != 0)
            printf("Return code from MQTT publish is %d\n", rc);
#if !defined(MQTT_TASK)
        if ((rc = MQTTYield(&client, 1000)) != 0)
            printf("Return code from yield is %d\n", rc);
#endif
    }
}


void StartMqttTask(void)
{
    osThreadAttr_t attr = {0};
    attr.name = "MqttEchoTask";
    attr.stack_size = 10240;
    attr.priority = osPriorityNormal;

    if (osThreadNew(MqttEchoTask, NULL, &attr) == NULL) {
        printf("create MqttEchoTask failed!\n");
    }
}
