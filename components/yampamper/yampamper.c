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

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "esp_system.h"
#include "esp_log.h"

#include "MQTTClient.h"

#include "yampamper.h"
static const char *TAG = "yampamper";

static yampamper_type_t type;
static bool connected = false;
static uint32_t pub_counter = 0;
static char clientId[UUID_LEN];
MQTTClient client;
Network *network;
unsigned char sendbuf[YAMPAMPER_MAX_MESSAGE_LEN + 50], readbuf[YAMPAMPER_MAX_MESSAGE_LEN + 50] = {0};

void uuid(char buf[UUID_LEN])
{
  char c;
  for (int i = 0; i < UUID_LEN - 1; i += 1)
  {
    if (i == 8 || i == 13 || i == 18 || i == 23)
    {
      c = '-';
    }
    else if (i == 14)
    {
      c = '4';
    }
    else
    {
      // 0-F
      c = (uint8_t)esp_random() & 0x0f;
      c += 48;
      if (c >= 58)
        c += 39;
    }
    buf[i] = c;
  }
  buf[UUID_LEN - 1] = '\0';
}

int y_connect(yampamper_type_t _type, Network *_network, char *mqtt_broker, int mqtt_port)
{
  if (connected)
  {
    ESP_LOGE(TAG, "Already connected");
    return 0;
  }
  connected = true;
  type = _type;
  network = _network;
  int rc = 0;
  MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;

  MQTTClientInit(&client, network, 30000, sendbuf, sizeof(sendbuf), readbuf, sizeof(readbuf));

  char *address = mqtt_broker;

  if ((rc = NetworkConnect(network, address, mqtt_port)) != 0)
  {
    ESP_LOGI(TAG, "Return code from network connect is %d", rc);
    return rc;
  }

#if defined(MQTT_TASK)

  if ((rc = MQTTStartTask(&client)) != pdPASS)
  {
    ESP_LOGE(TAG, "Return code from start tasks is %d", rc);
    return rc;
  }
  else
  {
    ESP_LOGI(TAG, "Use MQTTStartTask");
  }

#endif

  connectData.MQTTVersion = 3;
  connectData.clientID.cstring = "ESP8266_yampamper";

  if ((rc = MQTTConnect(&client, &connectData)) != 0)
  {
    ESP_LOGE(TAG, "Return code from MQTT connect is %d", rc);
    return rc;
  }
  else
  {
    ESP_LOGI(TAG, "MQTT Connected");
  }

  return 0;
}

void y_to_str(char *data, char *str)
{
  if (strlen(data) >= YAMPAMPER_MAX_DATA_LEN)
  {
    ESP_LOGE(TAG, "y_to_str(): data is too long");
    return;
  }

  snprintf(str,
           YAMPAMPER_MAX_MESSAGE_LEN,
           "{\"data\":%s,\"meta\":{\"counter\":%lu,\"clientId\":\"%s\"}}",
           data,
           (unsigned long)pub_counter,
           clientId);
}

void y_subscribe(messageHandler messageHandler)
{
  if (!connected)
  {
    ESP_LOGE(TAG, "y_subscribe(): Need to connect first");
    return;
  }
  if (type != SUBSCRIBER)
  {
    ESP_LOGE(TAG, "y_subscribe(): Only subscribers can subscribe");
    return;
  }

  int rc = 0;
  if ((rc = MQTTSubscribe(&client, CONFIG_MQTT_TOPIC, 2, messageHandler)) != 0)
  {
    ESP_LOGE(TAG, "Return code from MQTT subscribe is %d", rc);
  }
  else
  {
    ESP_LOGI(TAG, "MQTT subscribe to topic '%s'", CONFIG_MQTT_TOPIC);
  }
}

void y_publish(char *data)
{
  if (!connected)
  {
    ESP_LOGE(TAG, "y_publish(): Need to connect first");
    return;
  }
  if (type != PUBLISHER)
  {
    ESP_LOGE(TAG, "y_publish(): Only publishers can publish");
    return;
  }

  if (pub_counter == 0)
  {
    uuid(clientId);
  }
  char payload[YAMPAMPER_MAX_MESSAGE_LEN];
  y_to_str(data, payload);

  pub_counter += 1;

  MQTTMessage message;

  message.qos = QOS2;
  message.retained = 0;
  message.payload = payload;
  message.payloadlen = strlen(payload);

  int rc = 0;
  if ((rc = MQTTPublish(&client, CONFIG_MQTT_TOPIC, &message)) != 0)
  {
    ESP_LOGE(TAG, "Return code from MQTT publish is %d", rc);
  }
  ESP_LOGD(TAG, "Published '%s' on %s", (char *)message.payload, CONFIG_MQTT_TOPIC);
}
