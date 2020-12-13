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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ohos_init.h"
#include "wifiiot_at.h"
#include "mqtt_echo.h"

static unsigned int MqttTestCmd(int argc, const char **argv)
{
    printf("%s @ %s +%d\r\n", __FUNCTION__, __FILE__, __LINE__);
    if (argc < 2) {
        printf("Usage: AT+MQTT=host,port[clientId,username,password]\r\n");
        printf("FAIL\r\n");
        return 1;
    }

    printf("argc = %d, argv =\r\n", argc);
    for (int i = 0; i < argc; i++) {
        printf("argv[%d] = %s\r\n", i, argv[i]);
    }

    const char* host = argv[0];
    unsigned short port = atoi(argv[1]);
    printf("MQTT test with %s %d start.\r\n", host, port);

    const char* clientId = argc > 2 ? argv[2] : "EchoClient";
    const char* username = argc > 3 ? argv[3] : NULL;
    const char* password = argc > 4 ? argv[4] : NULL;
    printf("clientId = %s\r\n", clientId);
    printf("username = %s\r\n", username);
    printf("password = %s\r\n", password);

    if (MqttEchoConnect(host, port, clientId, username, password) != 0) {
        printf("MqttEchoConnect failed!\r\n");
        printf("ERROR\r\n");
        return 1;
    }

    int rc = MqttEchoTest("OHOS/sample/a");
    if (rc != 0) {
        printf("ERROR\r\n");
        return 1;
    }

    MqttEchoDisconnect();

    printf("OK\r\n");
    return 0;
}

void MqttAtEntry(void)
{
    static AtCmdTbl cmdTable = {0};
    cmdTable.atCmdName = "+MQTT";
    cmdTable.atCmdLen = strlen(cmdTable.atCmdName);
    cmdTable.atSetupCmd = MqttTestCmd;

    MqttEchoInit();

    if (AtRegisterCmd(&cmdTable, 1) != 0) {
        printf("AtRegisterCmd failed!\r\n");
    }
}
SYS_RUN(MqttAtEntry);
