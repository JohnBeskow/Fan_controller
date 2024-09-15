#ifndef GAP_H
#define GAP_H

#include <stdint.h>

// Function prototypes
void ble_app_advertise(void);
int ble_gap_event(struct ble_gap_event *event, void *arg);
void ble_app_on_sync(void);
void host_task(void *param);
void Ble_task(void *pvParameters);

extern uint8_t ble_addr_type;

#endif // GAP_H
