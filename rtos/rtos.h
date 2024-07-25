#ifndef U_RTOS_H_
#define U_RTOS_H_

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "queue.h"

extern QueueHandle_t analog_stick_queue, nrf24_state_queue;

void threads_init(void);
void queues_init(void);
void kernel_start(void);

#endif
