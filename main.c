/******************************************************************************
 * File Name: main.c
 *
 * Description: This code example features a 5-segment CapSense slider and two
 *              CapSense buttons. Button 0 turns the LED ON, Button 1 turns the
 *              LED OFF, and the slider controls the brightness of the LED. The
 *              code example also features interfacing with Tuner GUI using I2C
 *              interface.
 *
 *              This code example uses FreeRTOS. For documentation and API
 *              references of FreeRTOS, visit : https://www.freertos.org
 *
 * Related Document: See README.md
 *
 *
 *******************************************************************************
 * Copyright 2019-2024, Cypress Semiconductor Corporation (an Infineon company) or
 * an affiliate of Cypress Semiconductor Corporation.  All rights reserved.
 *
 * This software, including source code, documentation and related
 * materials ("Software") is owned by Cypress Semiconductor Corporation
 * or one of its affiliates ("Cypress") and is protected by and subject to
 * worldwide patent protection (United States and foreign),
 * United States copyright laws and international treaty provisions.
 * Therefore, you may use this Software only as provided in the license
 * agreement accompanying the software package from which you
 * obtained this Software ("EULA").
 * If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
 * non-transferable license to copy, modify, and compile the Software
 * source code solely for use in connection with Cypress's
 * integrated circuit products.  Any reproduction, modification, translation,
 * compilation, or representation of this Software except as specified
 * above is prohibited without the express written permission of Cypress.
 *
 * Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
 * reserves the right to make changes to the Software without notice. Cypress
 * does not assume any liability arising out of the application or use of the
 * Software or any product or circuit described in the Software. Cypress does
 * not authorize its products for use in any products where a malfunction or
 * failure of the Cypress product may reasonably be expected to result in
 * significant property damage, injury or death ("High Risk Product"). By
 * including Cypress's product in a High Risk Product, the manufacturer
 * of such system or application assumes all risk of such use and in doing
 * so agrees to indemnify Cypress against all liability.
 *******************************************************************************/

//clang-format off
#include "FreeRTOS.h"
#include "capsense_task.h"
#include "cy_retarget_io.h"
#include "cyberon_asr.h"
#include "cybsp.h"
#include "cyhal.h"
#include "led_task.h"
#include "queue.h"
#include "stdlib.h"
#include "task.h"
#include "vc.h"
//clang-format on
/*******************************************************************************
 * Global constants
 ******************************************************************************/
/* Priorities of user tasks in this project. configMAX_PRIORITIES is defined in
 * the FreeRTOSConfig.h and higher priority numbers denote high priority tasks.
 */
#define TASK_CAPSENSE_PRIORITY (configMAX_PRIORITIES - 1)
#define TASK_LED_PRIORITY (configMAX_PRIORITIES - 2)

/* Stack sizes of user tasks in this project */
#define TASK_CAPSENSE_STACK_SIZE (256u)
#define TASK_LED_STACK_SIZE (configMINIMAL_STACK_SIZE)

/* Queue lengths of message queues used in this project */
#define SINGLE_ELEMENT_QUEUE (1u)

/*******************************************************************************
 * voice command
 ******************************************************************************/
#define FRAME_SIZE (480u)
#define SAMPLE_RATE_HZ (16000u)
#define DECIMATION_RATE (96u)
#define AUDIO_SYS_CLOCK_HZ (24576000u)
#define PDM_DATA (P10_5)
#define PDM_CLK (P10_4)
/*******************************************************************************
 * Function Name: main()
 ********************************************************************************
 * Summary:
 *  System entrance point. This function sets up user tasks and then starts
 *  the RTOS scheduler.
 *
 * Return:
 *  int
 *
 *******************************************************************************/
int main(void) {
  cy_rslt_t result;

  /* Initialize the device and board peripherals */
  result = cybsp_init();
  /* Board init failed. Stop program execution */
  if (result != CY_RSLT_SUCCESS) {
    CY_ASSERT(0);
  }

  /* Enable global interrupts */
  __enable_irq();

  /* Create the queues. See the respective data-types for details of queue
   * contents
   */
  vc_init();
  led_command_data_q = xQueueCreate(SINGLE_ELEMENT_QUEUE, sizeof(led_command_data_t));
  capsense_command_q = xQueueCreate(SINGLE_ELEMENT_QUEUE, sizeof(capsense_command_t));
  voice_command_q = xQueueCreate(SINGLE_ELEMENT_QUEUE, sizeof(voice_command_t));
  /* Create the user tasks. See the respective task definition for more
   * details of these tasks.
   */
  xTaskCreate(task_capsense, "CapSense Task", TASK_CAPSENSE_STACK_SIZE, NULL, TASK_CAPSENSE_PRIORITY, NULL);
  xTaskCreate(task_led, "Led Task", TASK_LED_STACK_SIZE, NULL, TASK_LED_PRIORITY, NULL);
  xTaskCreate(vc_task, "vc task", TASK_CAPSENSE_STACK_SIZE, NULL, CYHAL_ISR_PRIORITY_DEFAULT, NULL);
  /* Start the RTOS scheduler. This function should never return */
  vTaskStartScheduler();

  /********************** Should never get here ***************************/
  /* RTOS scheduler exited */
  /* Halt the CPU if scheduler exits */
  CY_ASSERT(0);

  for (;;) {
  }
}

/* [] END OF FILE  */
