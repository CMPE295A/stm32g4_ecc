#include "interface_mqtt.h"
#include "interface_at_command.h"

#include <string.h>

enum
{
	MQTT_STATE_IDLE = 0,
	MQTT_STATE_PUBLISH,
	MQTT_STATE_CHANGE_TOPIC,

	NUM_MQTT_STATES
};

static uint8_t state = MQTT_STATE_IDLE;
static uint8_t topic = MQTT_TOPIC_KEY;
static uint8_t new_topic = MQTT_TOPIC_KEY;
static uint16_t pub_attempt = 0;
static uint16_t topic_attempt = 0;
static char pub_buffer[MAX_MQTT_PUB_BUFFER_BYTES] = {0};
static char sub_buffer[MAX_MQTT_PUB_BUFFER_BYTES] = {0};

void mqtt__process(void)
{
	switch(state)
	{
	case MQTT_STATE_IDLE:
		break;

	case MQTT_STATE_PUBLISH:
	{
		bool success = at_interface__publish_string(pub_buffer);
		if(success || (pub_attempt++ > MQTT_PUBLISH_TIMEOUT_MS))
		{
			state = MQTT_STATE_IDLE;
			pub_attempt = 0;
		}
		break;
	}

	case MQTT_STATE_CHANGE_TOPIC:
	{
		bool success = at_interface__set_topic(new_topic);
		if(success || topic_attempt++ > MQTT_PUBLISH_TIMEOUT_MS)
		{
			topic = new_topic;
			state = MQTT_STATE_IDLE;
			topic_attempt = 0;
		}
	}
	}
}

uint8_t mqtt__get_current_topic(void)
{
	return topic;
}

bool mqtt__publish(char *message, uint16_t byte_length)
{
	bool registered = false;
	if(state == MQTT_STATE_IDLE)
	{
		if(byte_length < (MAX_MQTT_PUB_BUFFER_BYTES - 1))
		{
			memcpy(pub_buffer, message, byte_length);
			pub_buffer[byte_length+1] = '\0';
			state = MQTT_STATE_PUBLISH;
			registered = true;
		}
	}
	return registered;
}

bool mqtt__change_topic(uint8_t proposed_new_topic)
{
	bool success = false;
	if(state == MQTT_STATE_IDLE && proposed_new_topic < NUM_MQTT_TOPICS)
	{
		new_topic = proposed_new_topic;
		state = MQTT_STATE_CHANGE_TOPIC;
		success = true;
	}
	return success;
}

void mqtt__sub_set(char *message, uint16_t byte_length)
{
	if (byte_length < MAX_MQTT_PUB_BUFFER_BYTES)
	{
		memcpy(sub_buffer, message, byte_length);
	}
}

uint16_t mqtt__get_sub_message(char *message, uint16_t max_bytes)
{
	uint16_t byte_length = strlen(sub_buffer);
	if((byte_length < max_bytes) && (byte_length > 0))
	{
		memcpy(message, sub_buffer, byte_length);
	}
	return byte_length;
}
