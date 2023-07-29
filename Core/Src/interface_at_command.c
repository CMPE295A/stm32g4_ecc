/**
 * This interface monitors the UART driver for new characters
 * from the esp8266 and places them in a packet buffer
 */

#include "interface_at_command.h"
#include "driver_uart.h"
#include <string.h>
#include <stdio.h>

#define AT_MODEM_UART DRIVER_UART1
#define LINE_BUFFER_MAX 5
#define LINE_MAX_LEN 512

static const char DRONE_ID[] = "1";
static const char EMPTY_LINE[] = "\0";
static const char AT_CMD_ECHO_OFF[] = "ATE0\r\n";
static const char AT_CMD_TEST[] = "AT\r\n";
static const char AT_CMD_MODE[] = "AT+CWMODE_CUR=1\r\n";
static const char AT_CMD_LIST_AP[] = "AT+CWLAP\r\n";
//static const char AT_CMD_SSID[] = "AT+CWJAP_CUR=\"MyBaloneysFirstName\",\"BernieLocke\",\"ce:9e:43:c4:13:ed\"\r\n";
static const char AT_CMD_SSID[] = "AT+CWJAP_CUR=\"scott_hotspot\",\"20jack23\",\"02:5d:52:2f:07:b5\"\r\n";
static const char AT_CMD_DHCP[] = "AT+CWDHCP_CUR=1,1\r\n";

static const char AT_CMD_MQTT_SETUP[] = "AT+MQTTSET=\"username\",\"passwd\",\"D1\",60\r\n";
static const char AT_CMD_MQTT_TOPIC_STATUS[] = "AT+MQTTTOPIC=\"drone/1/status\",\"SUB_TOPIC\"";
static const char AT_CMD_MQTT_TOPIC_GPS[] = "AT+MQTTTOPIC=\"drone/1/gps\",\"SUB_TOPIC\"";
static const char AT_CMD_MQTT_TOPIC_BATTERY[] = "AT+MQTTTOPIC=\"drone/1/battery\",\"SUB_TOPIC\"";
static const char AT_CMD_MQTT_TOPIC_TEMPERATURE[] = "AT+MQTTTOPIC=\"drone/1/temperature\",\"SUB_TOPIC\"";
static const char AT_CMD_MQTT_CONNECT[] = "AT+MQTTCON=0,\"192.168.0.160\",1883";
static const char AT_CMD_MQTT_PULBLISH[] = "AT+MQTTPUB=";
static const char AT_CMD_MQTT_TOPIC_PUBKEY[] = "AT+MQTTTOPIC=\"drone/1/pubKey\",\"SUB_TOPIC\"";


//static const char AT_CMD_SSID[] = "AT+CWJAP=\"Scott's iPhone\",\"MarlinBrando\"\r\n";
static const char AT_CMD_CONNECT[] = "AT+CIPSTART=\"UDP\",\"192.168.73.160\",50103,50108,0\r\n";
static const char AT_CMD_SEND_PREFIX[] = "AT+CIPSEND=";
static const char AT_CMD_SEND_TEST[] = "AT+CIPSEND=12\r\n";

static const char AT_RESPONSE_OK[] = "OK";
static const char AT_RESPONSE_SEND_OK[] = "SEND_OK";

static const char AT_CMD_ENDING[] = "\r\n";

static const char TEST_STRING1[] = "HELLO WORLD1";
static const char TEST_STRING2[] = "HELLO WORLD2";

static uint16_t state = AT_INTERFACE_ECHO_OFF;

static void read(void);
static bool line_is(const char *str);
static void send_at_cmd(const char *cmd);

static char line_buffer[LINE_BUFFER_MAX][LINE_MAX_LEN];
static char *line = &line_buffer[0][0];
static char *last_line = (char*)EMPTY_LINE;
static uint16_t next_line = 1;
static uint16_t char_index = 0;
static uint32_t send_count = 0;



bool at_interface__initialize(void)
{
	// connect to esp8266
	bool initialized = uart__initialize(AT_MODEM_UART, 115200);
	send_at_cmd(AT_CMD_ECHO_OFF);
//	send_at_cmd(AT_CMD_TEST);
	return initialized;
}

void at_interface__process(void)
{
	// called at user level fast loop
	read();

	switch(state)
	{
	case AT_INTERFACE_ECHO_OFF:
		if(line_is(AT_RESPONSE_OK))
		{
			send_at_cmd(AT_CMD_TEST);
			state = AT_INTERFACE_TEST;
		}
		break;

	case AT_INTERFACE_TEST:
		if(line_is(AT_RESPONSE_OK))
		{
			send_at_cmd(AT_CMD_MODE);
			state = AT_INTERFACE_MODE;
		}
		break;

	case AT_INTERFACE_MODE:
		if(line_is(AT_RESPONSE_OK))
		{
			send_at_cmd(AT_CMD_DHCP);
			state = AT_INTERFACE_SET_DHCP;
		}
		break;

	case AT_INTERFACE_SET_DHCP:
		if(line_is(AT_RESPONSE_OK))
		{
			send_at_cmd(AT_CMD_LIST_AP);
			state = AT_INTERFACE_LIST_AP;
		}
		break;

	case AT_INTERFACE_LIST_AP:
		if(line_is(AT_RESPONSE_OK))
		{
			send_at_cmd(AT_CMD_SSID);
			state = AT_INTERFACE_SSID;
		}
		break;

	case AT_INTERFACE_SSID:
		if(line_is(AT_RESPONSE_OK))
		{
			send_at_cmd(AT_CMD_CONNECT);
			state = AT_INTERFACE_CONNECT;
		}
		break;

	case AT_INTERFACE_CONNECT:
		if(line_is(AT_RESPONSE_OK))
		{
			send_at_cmd(AT_CMD_MQTT_SETUP);
			state = AT_INTERFACE_MQTT_SETUP;
		}
		break;

	case AT_INTERFACE_MQTT_SETUP:
		if(line_is(AT_RESPONSE_OK))
		{
			send_at_cmd(AT_CMD_MQTT_CONNECT);
			state = AT_INTERFACE_MQTT_CONNECT;
		}

	case AT_INTERFACE_MQTT_CONNECT:
		if(line_is(AT_RESPONSE_OK))
		{
			state = AT_INTERFACE_NETWORK_UP;
		}

	case AT_INTERFACE_NETWORK_UP:
		if(line_is(AT_RESPONSE_SEND_OK))
		{
			send_count++;
		}
		break;

	case AT_INTERFACE_FAULT:
		break;
	}
}

void at_interface_get_packet(char *packet, uint16_t size)
{
	// pull last packet from buffer
}

void at_interface__send_test_string(void)
{
	static bool init = true;
	static bool one = true;
	if(state == AT_INTERFACE_NETWORK_UP)
	{
		if(init)
		{
			send_at_cmd(AT_CMD_SEND_TEST);
			init = false;
		}
		else
		{
			if(one)
			{
				send_at_cmd(TEST_STRING1);
				one = false;
			}
			else
			{
				send_at_cmd(TEST_STRING2);
				one = true;
			}
			init = true;
		}
	}
}

void at_interface__send_string(char *str)
{
	if(state == AT_INTERFACE_NETWORK_UP)
	{
		uint16_t prefix_len = strlen(AT_CMD_SEND_PREFIX);
		uint16_t suffix_len = strlen(AT_CMD_ENDING);
		uint16_t str_len = strlen(str);
		if(str_len < (LINE_MAX_LEN - (prefix_len + suffix_len)))
		{
			char cmd_string[LINE_MAX_LEN] = {0};
			memcpy(cmd_string, AT_CMD_SEND_PREFIX, prefix_len);

			char len_str[5] = {0};
			sprintf(len_str, "%d", str_len);
			memcpy(&cmd_string[prefix_len], len_str, strlen(len_str));
			prefix_len += strlen(len_str);
			memcpy(&cmd_string[prefix_len], AT_CMD_ENDING, suffix_len);
			send_at_cmd(cmd_string);

			char string_to_send[LINE_MAX_LEN] = {0};
			string_to_send[0] = '>';
			memcpy(&string_to_send[1], str, strlen(str));
			send_at_cmd(string_to_send);
		}
	}
}

static bool line_is(const char *str)
{
	bool match = false;
	if(strcmp(str, last_line) == 0)
	{
		last_line = (char*)EMPTY_LINE;
		match = true;
	}
	return match;
}

static void read(void)
{
	char c = (char)uart__get_char(AT_MODEM_UART);
	while(c != NO_CHAR_AVAILABLE)
	{
		if(c != '\r' && c != '\n')
		{
			if(char_index != (LINE_MAX_LEN-1))
			{
				line[char_index] = c;
				char_index++;
			}
		}
		else
		{
			if(char_index != 0)
			{
				line[char_index] = '\0';
				last_line = line;
				line = &line_buffer[next_line][0];
				if(++next_line == LINE_BUFFER_MAX)
				{
					next_line = 0;
				}
				char_index = 0;
			}
		}
		c = uart__get_char(AT_MODEM_UART);
	}
}

static void send_at_cmd(const char *cmd)
{
	uint16_t len = strlen(cmd);
	uart__put(AT_MODEM_UART, (uint8_t*)cmd, len);
}
