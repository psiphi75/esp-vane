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

#ifndef __YAMPAMPER_H
#define __YAMPAMPER_H

#include "MQTTClient.h"

#define UUID_LEN (38)
#define YAMPAMPER_MAX_DATA_LEN (127)
#define YAMPAMPER_MAX_MESSAGE_LEN (255)

/**
 * Meta data used to describe the message ordering and which client it came from.
 */
typedef struct yampamper_meta_t
{
  uint64_t counter;        // The nth message sent
  char clientId[UUID_LEN]; // The clientId, this may change
} yampamper_meta_t;

/**
 * A message encapusalated within a MQTT message.
 */
typedef struct yampamper_msg_t
{
  char data[YAMPAMPER_MAX_MESSAGE_LEN]; // The information the client sends/receives
  struct yampamper_meta_t meta;         // Information that the client does not see
} yampamper_msg_t;

typedef enum
{
  PUBLISHER,
  SUBSCRIBER
} yampamper_type_t;

void y_publish(char *data);
int y_connect(yampamper_type_t _type, Network *_network, char *mqtt_broker, int mqtt_port);
void y_subscribe(messageHandler messageHandler);

#endif
