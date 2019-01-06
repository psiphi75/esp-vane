/*****************************************************************************
 *                                                                           *
 *  Copyright 2018 Simon M. Werner                                           *
 *                                                                           *
 *  Licensed under the Apache License, Version 2.0 (the "License");          *
 *  you may not use this file except in compliance with the License.         *
 *  You may obtain a copy of the License at                                  *
 *                                                                           *
 *      http://www.apache.org/licenses/LICENSE-2.0                           *
 *                                                                           *
 *  Unless required by applicable law or agreed to in writing, software      *
 *  distributed under the License is distributed on an "AS IS" BASIS,        *
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. *
 *  See the License for the specific language governing permissions and      *
 *  limitations under the License.                                           *
 *                                                                           *
 *****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_log.h"
#include "esp_system.h"
#include "esp_err.h"
#include "esp_task_wdt.h"

#include "../components/yampamper/yampamper.h"
#include "../components/mpu9250/common.h"

#include "imu.h"
#include "comms.h"

static const char *TAG = "main";

#define I2C_MASTER_NUM I2C_NUM_0 /*!< I2C port number for master dev */

// FIXME: `json_data` is shared across two threads, this may blow up every once in while.
char json_data[YAMPAMPER_MAX_DATA_LEN];
static void imu_task(void *arg)
{
  static uint64_t i = 1;

  imu_init();

  while (true)
  {
    imu_run();

    if (i++ % 50 == 0)
    {
      imu_get_data(json_data);
    }

    // Make the WDT happy
    esp_task_wdt_reset();
    pause();
  }

  // Exit
  vTaskDelete(NULL);
}

static void comms_task(void *arg)
{
  if (comms_connect() != 0)
  {
    ESP_LOGW(TAG, "Rebooting, because there was an error with the connection");
    vTaskDelay(1000 / portTICK_RATE_MS);
    esp_restart();
  }

  while (true)
  {
    publish(json_data);
  }

  // Exit
  vTaskDelete(NULL);
}

void app_main(void)
{
  comms_init();
  xTaskCreate(comms_task, "comms_task", 1024 * 10, NULL, 10, NULL);

  vTaskDelay(1000 / portTICK_RATE_MS);
  xTaskCreate(imu_task, "imu_task", 1024 * 4, NULL, 10, NULL);
}
