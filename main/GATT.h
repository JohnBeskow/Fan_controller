#ifndef GATT_H
#define GATT_H

#include <stdint.h>
#include "host/ble_hs.h"

// Max lengths for credentials
#define MAX_SSID_LENGTH 40
#define MAX_PASSWORD_LENGTH 32
#define MAX_PORT_LENGTH 10
#define MAX_IP_LENGTH 20

// Default credentials
extern char SSID[MAX_SSID_LENGTH];
extern char PASSWORD[MAX_PASSWORD_LENGTH];
extern char PORT[MAX_PORT_LENGTH];
extern char IP[MAX_IP_LENGTH];

// Function prototypes
static int ble_write_handler(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg);
static int parse_received_data(const char *data);

extern const struct ble_gatt_svc_def gatt_svcs[];

#endif // GATT_H
