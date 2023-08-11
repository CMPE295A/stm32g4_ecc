#pragma once
#include "stdbool.h"
#include "stdint.h"

#define MAX_MQTT_PUB_BUFFER_BYTES 512
#define MQTT_PUBLISH_TIMEOUT_MS (10 * 1000)

enum
{
	MQTT_TOPIC_KEY,
	MQTT_TOPIC_DEFUALT,

	NUM_MQTT_TOPICS
};

//bool connect(void);
void mqtt__process(void);
bool mqtt__publish(char *message, uint16_t byte_length);
void mqtt__sub_set(char *message, uint16_t byte_length);
uint16_t mqtt__get_sub_message(char *message, uint16_t max_bytes);
bool mqtt__change_topic(uint8_t new_topic);
uint8_t mqtt__get_current_topic(void);
