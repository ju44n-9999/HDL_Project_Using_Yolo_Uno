#include "global.h"
#include "global_semaphore.h"

#include "led_blinky.h"
#include "neo_blinky.h"
#include "temp_humi_monitor.h"
#include "fan_control.h"
// #include "mainserver.h"
// #include "tinyml.h"
#include "coreiot.h"

// include task
#include "task_check_info.h"
#include "task_toogle_boot.h"
#include "task_wifi.h"
// #include "task_webserver.h"
// #include "task_core_iot.h"

void setup()
{
  Serial.begin(115200);
  Wire.begin(11, 12);
  
  fan_init();  // Initialize fan - always off
  check_info_File(0);
  init_semaphores();

  xTaskCreate(led_blinky, "Task LED Blink", 2048, NULL, 2, NULL);
  xTaskCreate(neo_blinky, "Task NEO Blink", 2048, NULL, 2, NULL);
  xTaskCreate(temp_humi_monitor, "Task TEMP HUMI Monitor", 2048, NULL, 2, NULL);
  xTaskCreate(fan_control_task, "Task Fan Control", 2048, NULL, 2, NULL);
  xTaskCreate(task_wifi_handler, "Task WiFi", 4096, NULL, 2, NULL);
  // xTaskCreate(main_server_task, "Task Main Server" ,8192  ,NULL  ,2 , NULL);
  // xTaskCreate(tiny_ml_task, "Tiny ML Task" ,2048  ,NULL  ,2 , NULL);
  xTaskCreate(coreiot_task, "CoreIOT Task" ,4096  ,NULL  ,2 , NULL);
  xTaskCreate(Task_Toogle_BOOT, "Task_Toogle_BOOT", 4096, NULL, 2, NULL);
  // xTaskCreate(task_webserver_update, "Task_WebServer", 8192, NULL, 2, NULL);
}

void loop()
{
  /*if (check_info_File(1))
  {
    if (!Wifi_reconnect())
    {
      Webserver_stop();
    }
    else
    {
      CORE_IOT_reconnect();
    }
  }
  Webserver_reconnect();*/
}