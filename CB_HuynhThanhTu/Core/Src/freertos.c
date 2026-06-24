/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "cmsis_os.h"
#include "lwip.h"
#include "lwip/api.h"
#include "mqtt_task.h"

#include "mbedtls_config.h"
#include "mqttinterface.h"
#include "mbedtls/error.h"

#include "config.h"


/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */


/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
extern struct netif gnetif;

 osThreadId mqttClientSubTaskHandle;  //mqtt client task handle
 osThreadId mqttClientPubTaskHandle;  //mqtt client task handle
 osThreadId defaultTaskHandle;
/* USER CODE END Variables */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void MqttClientSubTask(void const *argument); //mqtt client subscribe task function
void MqttClientPubTask(void const *argument); //mqtt client publish task function
int  MqttConnectBroker(void); 				//mqtt broker connect function
void MqttMessageArrived(MessageData* msg); //mqtt message callback function



void StartDefaultTask(void const * argument);
extern void MX_LWIP_Init(void);
void MX_FREERTOS_Init(void);
/* USER CODE END FunctionPrototypes */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* Hook prototypes */
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName);

/* USER CODE BEGIN 4 */
__weak void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName)
{
   /* Run time stack overflow checking is performed if
   configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2. This hook function is
   called if a stack overflow is detected. */
}
/* USER CODE END 4 */

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
	{
	  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
	  *ppxIdleTaskStackBuffer = &xIdleStack[0];
	  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
	  /* place for user code */
	}
void MX_FREERTOS_Init(void)
	{


		osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 1024);
		defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);



	}



void StartDefaultTask(void const * argument)
{
	MX_LWIP_Init();
	while(1)
		{
			//waiting for valid ip address
			if (gnetif.ip_addr.addr == 0 || gnetif.netmask.addr == 0 || gnetif.gw.addr == 0) //system has no valid ip address
			{
				printf("Waiting for IP... \n");
				osDelay(1000);
				continue;
			}
			else
			{
				printf("Got IP: %s\n", ip4addr_ntoa(&gnetif.ip_addr));
				printf("DHCP/Static IP O.K.\n");
				break;
			}

		}
	mbedtls_x509_crt_init(&cacert);


//	for (int i = 0; i < strlen(mbedtls_root_certificate); i++)
//	{
//	    printf("%02X ", mbedtls_root_certificate[i]);
//	}
	int ret = mbedtls_x509_crt_parse(&cacert, (const unsigned char *)mbedtls_root_certificate, strlen(mbedtls_root_certificate) + 1);
	if (ret != 0)
	{
		char err_buf[100];
		mbedtls_strerror(ret, err_buf, sizeof(err_buf));
		printf("�?� CA load failed: -0x%04X: %s\n", -ret, err_buf);
	}

	else
	{
	    printf("✅ CA loaded successfully!\n");

	}

	 osThreadDef(mqttClientSubTask, MqttClientSubTask, osPriorityNormal, 0, configMINIMAL_STACK_SIZE*4);
	 mqttClientSubTaskHandle = osThreadCreate(osThread(mqttClientSubTask), NULL);

	 osThreadDef(mqttClientPubTask, MqttClientPubTask, osPriorityNormal, 0, configMINIMAL_STACK_SIZE*4);
	 mqttClientPubTaskHandle = osThreadCreate(osThread(mqttClientPubTask), NULL);
//	 if (mqttClientSubTaskHandle == NULL)
//	 {
//	     printf("�?� Failed to create mqttClientSubTask\r\n");
//	 }
//	 else
//	 {
//	     printf("✅ mqttClientSubTask created\r\n");
//	 }
	 while(1)
	    {
	        osDelay(1000);  // Không làm gì, chỉ sống
	    }



}



/* USER CODE END GET_IDLE_TASK_MEMORY */

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */
