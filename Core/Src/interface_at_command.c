/**
 * This interface monitors the UART driver for new characters
 * from the esp8266 and places them in a packet buffer
 */

#include "interface_at_command.h"
#include "driver_uart.h"
#include "interface_mqtt.h"
#include "keys.h"
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
//static const char AT_CMD_SSID[] = "AT+CWJAP_CUR=\"MyBaloneysFirstName\",\"BernieLocke\",\"c8:9e:43:c3:88:26\"\r\n";
static const char AT_CMD_SSID[] = "AT+CWJAP_CUR=\"scott_hotspot\",\"20jack23\"\r\n";
static const char AT_CMD_DHCP[] = "AT+CWDHCP_CUR=1,1\r\n";
static const char AT_CMD_RESTORE[]= "AT+RESTORE=1\r\n";
static const char AT_CMD_SSL_CONFIG[] = "AT+CIPSSLCCONF=2\r\n";

static const char AT_CMD_CA_CERT_DELETE[] = "AT+CASEND=\r\n";
static const char AT_CMD_CA_CERT_SET[] = "AT+CASEND=1\r\n";
static const char AT_CMD_CLIENT_CERT_DELETE[] = "AT+CLICASEND=0\r\n";
static const char AT_CMD_CLIENT_CERT_SET[] = "AT+CLICASEND=1\r\n";
static const char AT_CMD_PRIVATE_KEY_DELETE[] = "AT+AWSPKSEND=0\r\n";
static const char AT_CMD_PRIVATE_KEY_SET[] = "AT+AWSPKSEND=1\r\n";

static const char AT_CMD_MQTT_SETUP[] = "AT+MQTTSET=\"scott\",\"scott\",\"D1\",60\r\n";
static const char AT_CMD_MQTT_TOPIC_STATUS[] = "AT+MQTTTOPIC=\"drone/1/status\",\"SUB_TOPIC\"\r\n";
static const char AT_CMD_MQTT_CONNECT[] = "AT+MQTTCON=0,\"192.168.1.12\",1883\r\n";
static const char AT_CMD_MQTT_PUBLISH_TEST[] = "AT+MQTTPUB=\"TESTING\"\r\n";
static const char AT_CMD_MQTT_PUBLISH_PREFIX[] = "AT+MQTTPUB=";

static const char AT_CMD_MQTT_TOPIC_STATUS_AWS[] = "AT+MQTTTOPIC=\"$aws/things/Microcontroller/shadow/name/shadow/update\",\"$aws/things/Microcontroller/shadow/name/shadow/update/accepted\"\r\n";
static const char AT_RESPONSE_MQTT_SUB_TOPIC[] = "$aws/things/Microcontroller/shadow/name/shadow/update/accepted -> ";
static const char AT_CMD_MQTT_SETUP_AWS[] = "AT+MQTTSET=\"\",\"\",\"Microcontroller\",60\r\n";
static const char AT_CMD_AWS_CONNECT[] = "AT+AWSCON=\"a3vvj2kk3rs3as-ats.iot.us-west-1.amazonaws.com\"\r\n";

static const char AT_CMD_CONNECT[] = "AT+CIPSTART=\"UDP\",\"192.168.73.160\",50103,50108,0\r\n";
static const char AT_CMD_SEND_PREFIX[] = "AT+CIPSEND=\"";
static const char AT_CMD_SEND_TEST[] = "AT+CIPSEND=12\r\n";

static const char AT_RESPONSE_OK[] = "OK";
static const char AT_RESPONSE_GOT_IP[] = "WIFI GOT IP";
static const char AT_RESPONSE_SEND_OK[] = "SEND_OK";
static const char AT_RESPONSE_READY[] = "ready";
static const char AT_RESPONSE_CLOSED[] = "CLOSED";
static const char AT_RESPONSE_LINK_INVALID[] = "link is not valid";

static const char AT_CMD_ENDING[] = "\r\n";

static const char TEST_STRING1[] = "HELLO WORLD1";
static const char TEST_STRING2[] = "HELLO WORLD2";

static uint16_t state = AT_INTERFACE_INIT;
static uint16_t last_state = AT_INTERFACE_INIT;

static void read(void);
static bool line_is(const char *str);
static bool last_line_empty(void);
static bool next_to_last_line_is(const char *str);
static void send_at_cmd(const char *cmd);
static void transition(const char *next_cmd, uint8_t next_state);
static void step(const char *response, const char *next_command, uint8_t next_state);
static void step_multiline(int size, const char next_command[][KEY_MAX_LINE], uint8_t next_state);

static char line_buffer[LINE_BUFFER_MAX][LINE_MAX_LEN];
static char *line = &line_buffer[0][0];
static char *last_line = (char*)EMPTY_LINE;
static char *next_to_last_line = (char*)EMPTY_LINE;
static uint16_t next_line = 1;
static uint16_t char_index = 0;
static uint32_t send_count = 0;
static bool ms_tick = 0;
static bool start_transition = false;



bool at_interface__initialize(void)
{
	// connect to esp8266
	bool initialized = uart__initialize(AT_MODEM_UART, 115200);
	send_at_cmd(AT_CMD_RESTORE);
	state = AT_INTERFACE_INIT;
	return initialized;
}

void at_interface__process(bool ms_elapsed)
{
	// called at user level fast loop
	read();
	ms_tick = ms_elapsed;

	switch(state)
	{
	case AT_INTERFACE_INIT:
		step(AT_RESPONSE_READY, AT_CMD_TEST, AT_INTERFACE_TEST);
		break;

//	case AT_INTERFACE_ECHO_OFF:
//		step(AT_RESPONSE_OK, AT_CMD_TEST, AT_INTERFACE_TEST);
//		break;

	case AT_INTERFACE_TEST:
		step(AT_RESPONSE_OK, AT_CMD_MODE, AT_INTERFACE_MODE);
		break;

	case AT_INTERFACE_MODE:
		step(AT_RESPONSE_OK, AT_CMD_DHCP, AT_INTERFACE_SET_DHCP);
		break;

	case AT_INTERFACE_SET_DHCP:
		step(AT_RESPONSE_OK, AT_CMD_SSID, AT_INTERFACE_SSID);
		break;

//	case AT_INTERFACE_LIST_AP:
//		step(AT_RESPONSE_OK, AT_CMD_SSID, AT_INTERFACE_SSID);
//		break;

	case AT_INTERFACE_SSID:
		step(AT_RESPONSE_GOT_IP, AT_CMD_SSL_CONFIG, AT_INTERFACE_SSL_CONFIG);
		break;

	case AT_INTERFACE_SSL_CONFIG:
		step(AT_RESPONSE_OK, AT_CMD_CA_CERT_SET, AT_INTERFACE_CLA_CERT_SET);
//		step(AT_RESPONSE_OK, AT_CMD_PRIVATE_KEY_SET, AT_INTERFACE_PRIVATE_KEY_SET);
		break;

//	case AT_INTERFACE_CONNECT:
//		step(AT_RESPONSE_OK, AT_CMD_MQTT_SETUP, AT_INTERFACE_MQTT_SETUP);
//		break;

	case AT_INTERFACE_CLA_CERT_SET:
		step_multiline(CA_ROOT_CERT_NUM_LINES, CA_ROOT_CERT, AT_INTERFACE_CLA_CERT_SEND);
		break;

	case AT_INTERFACE_CLA_CERT_SEND:
		step(AT_RESPONSE_OK, AT_CMD_CLIENT_CERT_SET, AT_INTERFACE_CLIENT_CERT_SET);
		break;

	case AT_INTERFACE_CLIENT_CERT_SET:
		step_multiline(CLIENT_CERT_NUM_LINES, CLIENT_CERT, AT_INTERFACE_CLIENT_CERT_SEND);
		break;

	case AT_INTERFACE_CLIENT_CERT_SEND:
		step(AT_RESPONSE_OK, AT_CMD_PRIVATE_KEY_SET, AT_INTERFACE_PRIVATE_KEY_SET);
		break;

	case AT_INTERFACE_PRIVATE_KEY_SET:
		step_multiline(PRIVATE_KEY_NUM_LINES, PRIVATE_KEY, AT_INTERFACE_PRIVATE_KEY_SEND);
		break;

	case AT_INTERFACE_PRIVATE_KEY_SEND:
		step(AT_RESPONSE_OK, AT_CMD_MQTT_TOPIC_STATUS_AWS, AT_INTERFACE_MQTT_SET_TOPIC);
		break;

	case AT_INTERFACE_MQTT_SET_TOPIC:
		step(AT_RESPONSE_OK, AT_CMD_MQTT_SETUP_AWS, AT_INTERFACE_MQTT_SETUP);
		break;

	case AT_INTERFACE_MQTT_SETUP:
		step(AT_RESPONSE_OK, AT_CMD_AWS_CONNECT, AT_INTERFACE_MQTT_CONNECT);
		break;

	case AT_INTERFACE_MQTT_CONNECT:
		if(line_is(AT_RESPONSE_OK))
		{
			last_state = state;
			state = AT_INTERFACE_NETWORK_UP;
		}
		break;

	case AT_INTERFACE_NETWORK_UP:
		if(line_is(AT_RESPONSE_LINK_INVALID))
		{
			last_state = AT_INTERFACE_INIT;
			state = AT_INTERFACE_INIT;
			at_interface__initialize();
		}
		else if(next_to_last_line_is(AT_RESPONSE_MQTT_SUB_TOPIC))
		{
			last_state = state;
			state = AT_INTERFACE_GET_SHARED_SECRET;
		}
		break;

	case AT_INTERFACE_GET_SHARED_SECRET:
		if(!last_line_empty())
		{
			mqtt__sub_set(last_line, strlen(last_line));
			last_state = state;
			state = AT_INTERFACE_NETWORK_UP;
		}
		break;

	case AT_INTERFACE_MQTT_DISCONNECT:
		step(AT_RESPONSE_CLOSED, AT_CMD_MQTT_TOPIC_STATUS_AWS, AT_INTERFACE_MQTT_SET_TOPIC);
		break;

	case AT_INTERFACE_FAULT:
		break;
	}
}

void at_interface_get_packet(char *packet, uint16_t size)
{
	// pull last packet from buffer
}

void at_interface__publish_test(void)
{
	if(state == AT_INTERFACE_NETWORK_UP)
	{
		send_at_cmd(AT_CMD_MQTT_PUBLISH_TEST);
	}
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

bool at_interface__publish_string(char *str)
{
	bool sent = false;
	if(state == AT_INTERFACE_NETWORK_UP)
	{
		uint16_t prefix_len = strlen(AT_CMD_MQTT_PUBLISH_PREFIX);
		uint16_t suffix_len = strlen(AT_CMD_ENDING);
		uint16_t str_len = strlen(str);
		if(str_len < (LINE_MAX_LEN - (prefix_len + suffix_len)))
		{
			char cmd_string[LINE_MAX_LEN] = {0};
			memcpy(cmd_string, AT_CMD_MQTT_PUBLISH_PREFIX, prefix_len);
			uint16_t index = prefix_len;
			memcpy(&cmd_string[index], str, str_len);
			index += str_len;

			memcpy(&cmd_string[index], AT_CMD_ENDING, suffix_len);
			send_at_cmd(cmd_string);
			sent = true;
		}
	}
	return sent;
}

static bool line_is(const char *str)
{
	bool match = false;
	if(strcmp(str, last_line) == 0)
	{
		next_to_last_line = last_line;
		last_line = (char*)EMPTY_LINE;
		match = true;
	}
	return match;
}

static bool next_to_last_line_is(const char *str)
{
	bool match = false;
	if(strcmp(str, next_to_last_line) == 0)
	{
		match = true;
	}
	return match;
}

static bool last_line_empty(void)
{
	bool empty = false;
	if(strcmp(EMPTY_LINE, last_line) == 0)
	{
		empty = true;
	}
	return empty;
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
				next_to_last_line = last_line;
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

static void step(const char *response, const char *next_command, uint8_t next_state)
{
	if(response != NULL)
	{
		if(line_is(response) && !start_transition)
		{
			start_transition = true;
		}
	}
	else
	{
		start_transition = true;
	}

	if(start_transition)
	{
		transition(next_command, next_state);
	}
}

static void step_multiline(int size, const char next_command[][KEY_MAX_LINE], uint8_t next_state)
{
	static uint16_t send_delay_count = 0;
	static int i = 0;
	if(ms_tick)
	{
		send_delay_count++;
	}
	if(send_delay_count >= 10)
	{
		send_at_cmd(next_command[i]);
		if(++i == size)
		{
			last_state = state;
			state = next_state;
			send_delay_count = 0;
			start_transition = false;
			i = 0;
		}
		send_delay_count = 0;
	}
}

static void transition(const char *next_cmd, uint8_t next_state)
{
	static uint16_t send_delay_count = 0;

	if(ms_tick)
	{
		send_delay_count++;
	}

	if(send_delay_count > AT_INTERFACE_SEND_DELAY_MS)
	{
		send_at_cmd(next_cmd);
		last_state = state;
		state = next_state;
		send_delay_count = 0;
		start_transition = false;
	}
}
