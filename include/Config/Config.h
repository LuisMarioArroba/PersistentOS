#ifndef CONFIG_H
#define CONFIG_H

/*
GPIO Configuration
*/
#define LED_PIN             2
#define ONE_WIRE_BUS        4
#define TEST_BUTTON_PIN 15

/*
Scheduler Configuration
*/
#define MAX_TASKS           10
#define SYSTEM_TICK_MS      100

/*
Persistent Memory Configuration
*/

#define SENSOR_BUFFER_SIZE      32
#define ENERGY_HISTORY_SIZE     50
#endif