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

#ifndef __MQTT_HELPER_H
#define __MQTT_HELPER_H

#include "MQTTClient.h"

#define UUID_LEN (38)
#define MQTT_HELPER_MAX_DATA_LEN (127)
#define MQTT_HELPER_MAX_MESSAGE_LEN (255)

typedef enum
{
  PUBLISHER,
  SUBSCRIBER
} mqtt_helper_type_t;

int mqtt_connect(mqtt_helper_type_t _type, Network *_network, char *mqtt_broker, int mqtt_port);
int mqtt_publish(char *data);
void mqtt_subscribe(messageHandler messageHandler);

#endif
