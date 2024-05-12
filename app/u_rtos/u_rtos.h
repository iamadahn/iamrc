#ifndef U_RTOS_H_
#define U_RTOS_H_

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "queue.h"

extern QueueHandle_t analog_stick_queue;

void threads_init(void);

void led_controller_task(void* pv_argument);
void display_controller_task(void* pv_argument);
void analog_stick_controller_task(void* pv_argument);

#endif