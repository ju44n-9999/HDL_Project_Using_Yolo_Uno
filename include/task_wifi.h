#ifndef __TASK_WIFI_H__
#define __TASK_WIFI_H__

#include <WiFi.h>
#include <task_check_info.h>
#include <task_webserver.h>

extern bool Wifi_reconnect();
extern void startAP();
extern void task_wifi_handler(void *pvParameters);
extern void startSTA();

#endif