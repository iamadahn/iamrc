#ifndef U_RTOS_H_
#define U_RTOS_H_

void threads_init(void);

void led_controller_task(void* pv_argument);
void display_controller_task(void* pv_argument);

#endif