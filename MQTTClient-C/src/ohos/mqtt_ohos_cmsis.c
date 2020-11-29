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
#include "mqtt_ohos.h"

// static volatile int g_lastSeconds = 0;

#define MS_PER_S (1000)
#define US_PER_S (1000*1000)

int GetTimeVal(struct timeval* now)
{
    if (now == NULL) {
        return -1;
    }

    uint32_t tickPerSec = osKernelGetTickFreq();
    uint32_t sysPerSec = osKernelGetSysTimerFreq();
    uint32_t sysPerTick = sysPerSec / tickPerSec;

    uint32_t tickCount = osKernelGetTickCount();
    now->tv_sec = tickCount / tickPerSec;
    now->tv_usec = (tickCount % tickPerSec) * (US_PER_S / tickPerSec);

    uint64_t sysCount = osKernelGetSysTimerCount() % sysPerTick;
    now->tv_usec += sysCount * US_PER_S / sysPerSec;
    return 0;
}

#define gettimeofday(tv, tz) GetTimeVal(tv)

void TimerInit(Timer* timer)
{
    timer->endTime = (struct timeval){0, 0};
}

char TimerIsExpired(Timer* timer)
{
    struct timeval now, res;
    gettimeofday(&now, NULL);
    timersub(&timer->endTime, &now, &res);
    return res.tv_sec < 0 || (res.tv_sec == 0 && res.tv_usec <= 0);
}

void TimerCountdownMS(Timer* timer, unsigned int ms)
{
    struct timeval now;
    gettimeofday(&now, NULL);
    struct timeval interval = {ms / 1000, (ms % 1000) * 1000};
    timeradd(&now, &interval, &timer->endTime);
}

void TimerCountdown(Timer* timer, unsigned int seconds)
{
	struct timeval now;
	gettimeofday(&now, NULL);
	struct timeval interval = {seconds, 0};
	timeradd(&now, &interval, &timer->endTime);
}

int TimerLeftMS(Timer* timer)
{
    struct timeval now, res;
    gettimeofday(&now, NULL);
    timersub(&timer->endTime, &now, &res);
    return (res.tv_sec < 0) ? 0 : res.tv_sec * 1000 + res.tv_usec / 1000;
}


static int NetworkRead(Network* n, unsigned char* buffer, int len, int timeout_ms)
{
    return 0;
}


static int NetworkWrite(Network* n, unsigned char* buffer, int len, int timeout_ms)
{
    return 0;
}

void NetworkInit(Network* network)
{
    network->socket = -1;
    network->mqttread = NetworkRead;
    network->mqttwrite = NetworkWrite;
}

int NetworkConnect(Network* network, char* host, int port)
{
    return 0;
}

void NetworkDisconnect(Network* network)
{

}

#if defined(MQTT_TASK)
void MutexInit(Mutex* mutex)
{

}

int MutexLock(Mutex* mutex)
{
    return 0;
}

int MutexUnlock(Mutex* mutex)
{
    return 0;
}

int ThreadStart(Thread* thread, void (*fn)(void*), void* arg)
{

}
#endif
