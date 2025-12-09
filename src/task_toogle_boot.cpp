#include "task_toogle_boot.h"
#include "global.h"

#define BOOT 0

void Task_Toogle_BOOT(void *pvParameters)
{
    // Configure button as active-low with internal pull-up
    pinMode(BOOT, INPUT_PULLUP);

    unsigned long buttonPressStartTime = 0;
    while (true)
    {
        int state = digitalRead(BOOT);

        if (state == LOW)
        {
            if (buttonPressStartTime == 0)
            {
                buttonPressStartTime = millis();
            }
            else if (millis() - buttonPressStartTime > 2000)
            {
                // long press: delete config and restart (old behaviour)
                Delete_info_File();
                vTaskDelete(NULL);
            }
        }
        else
        {
            // button released; if it was previously pressed, evaluate duration
            if (buttonPressStartTime != 0)
            {
                unsigned long pressDuration = millis() - buttonPressStartTime;
                if (pressDuration < 2000)
                {
                    // short press: toggle DHT display mode
                    dht_display_mode = (dht_display_mode == 1) ? 2 : 1; // 1->2->1
                }
                // reset timer
                buttonPressStartTime = 0;
            }
        }

        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}