/*
 * mqtt_task.c
 *
 *  Created on: 2020. 5. 19.
 *      Author: eziya76@gmail.com
 */

#include "main.h"
#include "mqtt_task.h"
#include "mbedtls/ssl.h"
extern mbedtls_x509_crt cacert;



Network net;
MQTTClient mqttClient;
unsigned char sndBuffer[100];
unsigned char rcvBuffer[100];
unsigned char msgBuffer[100];
//mqtt subscribe task



void MqttClientSubTask(void const *argument)
{
	printf("✅ MqttClientSubTask started.\r\n");
	while(1)
	{
		if(!mqttClient.isconnected)
		{
			printf("❌ MQTT not connected\r\n");
			//try to connect to the broker
			MQTTDisconnect(&mqttClient);
			MqttConnectBroker();
			printf("---------HOAN THANH-------");
			osDelay(1000);
		}
		else
		{
			static uint8_t first_connect = 0;
			if (!first_connect)
			{
				first_connect = 1;
				printf("✅ MQTT connected\r\n");
			}

		    int rc = MQTTYieldNoTimer(&mqttClient, 1000);
		    if (rc == -0x004C || rc == MBEDTLS_ERR_SSL_WANT_READ)
		    {
		    	continue;
		    }
		    else if (rc != 0)
		    {
		    	first_connect = 0;
		        printf("⚠️ MQTT Yield error: %d, disconnecting...\r\n", rc);
		        MqttConnectBroker();
		    }
			osDelay(1);
		}

	}

}



//mqtt publish task
void MqttClientPubTask(void const *argument)
{
	printf("MqttClientPubTask started.\r\n");
	const char* str = "MQTT message from STM32";
	MQTTMessage message;

	while(1)
	{
		if(mqttClient.isconnected)
		{
			message.payload = (void*)str;
			message.payloadlen = strlen(str);

			MQTTPublish(&mqttClient, "stm32/test", &message); //publish a message
		}
		osDelay(10000);
	}
}

int MqttConnectBroker()
{
	int ret;

	printf("📡 MQTT CONNECTING...\r\n");
	printf("➡️  Broker: %s:%d\r\n", BROKER_IP, MQTT_PORT);

	net_clear();

	ret = net_init(&net);
	if(ret != MQTT_SUCCESS)
	{
		printf("❌1 net_init failed (%d)\r\n", ret);
		return -1;
	}

	ret = net_connect(&net, BROKER_IP, MQTT_PORT);
	if(ret != MQTT_SUCCESS)
	{
		printf("❌2 net_connect failed (%d)\r\n", ret);
		return -1;
	}

	 // Init MQTT
	MQTTClientInit(&mqttClient, &net, 2000, sndBuffer, sizeof(sndBuffer), rcvBuffer, sizeof(rcvBuffer));


	// Setup connect data
	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
	data.willFlag = 0;
	data.MQTTVersion = 3.1;
	data.clientID.cstring = "mqttSTM32";
	data.username.cstring = "combros";
	data.password.cstring = "Combros@123";
	data.keepAliveInterval = 60;
	data.cleansession = 1;

	printf("➡️  Sending MQTT CONNECT: clientID=%s, user=%s, keepAlive=%d, cleanSession=%d\n",
	           data.clientID.cstring, data.username.cstring,
	           data.keepAliveInterval, data.cleansession);

	// Send CONNECT
	printf("➡️  Sending MQTT CONNECT...\n");

	ret = MQTTConnect(&mqttClient, &data);

	printf("MQTTConnect return = %d\n", ret);

	if(ret != MQTT_SUCCESS)
	{
		printf("❌3 MQTTConnect failed (%d)\r\n", ret);
		return ret;
	}

	mqttClient.isconnected = 1;

	// Subscribe test topic
	ret = MQTTSubscribe(&mqttClient, "stm32/test", QOS0, MqttMessageArrived);
	if(ret != MQTT_SUCCESS)
	{
		printf("❌4 MQTTSubscribe failed (%d)\r\n", ret);
		return ret;
	}

	printf("✅ MqttConnectBroker OK!\r\n");
	return MQTT_SUCCESS;
}

void MqttMessageArrived(MessageData* msg)
{
//	HAL_GPIO_TogglePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin); //toggle pin when new message arrived

	MQTTMessage* message = msg->message;
	memset(msgBuffer, 0, sizeof(msgBuffer));
	memcpy(msgBuffer, message->payload,message->payloadlen);
	  printf("Topic   : %.*s\n",
	           msg->topicName->lenstring.len,
	           msg->topicName->lenstring.data);
	  printf("Payload : %.*s\n",
	             (int)msg->message->payloadlen,
	             (char*)msg->message->payload);

}
