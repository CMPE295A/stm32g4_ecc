#include "interface_mqtt.h"
#include "interface_at_command.h"

#include <string.h>

enum
{
	MQTT_STATE_IDLE = 0,
	MQTT_STATE_PUBLISH,

	NUM_MQTT_STATES
};

static uint8_t state = MQTT_STATE_IDLE;
static uint16_t pub_attempt = 0;
static char pub_buffer[MAX_MQTT_PUB_BUFFER_BYTES] = {0};

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
	}
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
