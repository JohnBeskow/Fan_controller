#include "GATT.h"
#include "esp_log.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include <string.h>

static const char *TAG = "BLE_GATT";

// Parsing received data
static int parse_received_data(const char *data) {
    char *token;
    char buffer[256];
    strncpy(buffer, data, sizeof(buffer) - 1);

    token = strtok(buffer, ",");
    if (token != NULL) {
        strncpy(SSID, token, MAX_SSID_LENGTH - 1);
        SSID[MAX_SSID_LENGTH - 1] = '\0';
    } else {
        ESP_LOGE(TAG, "SSID missing in the data");
        return -1;
    }

    token = strtok(NULL, ",");
    if (token != NULL) {
        strncpy(PASSWORD, token, MAX_PASSWORD_LENGTH - 1);
        PASSWORD[MAX_PASSWORD_LENGTH - 1] = '\0';
    } else {
        ESP_LOGE(TAG, "Password missing in the data");
        return -1;
    }

    token = strtok(NULL, ",");
    if (token != NULL) {
        strncpy(PORT, token, MAX_PORT_LENGTH - 1);
        PORT[MAX_PORT_LENGTH - 1] = '\0';
    } else {
        ESP_LOGE(TAG, "Port missing in the data");
        return -1;
    }

    token = strtok(NULL, ",");
    if (token != NULL) {
        strncpy(IP, token, MAX_IP_LENGTH - 1);
        IP[MAX_IP_LENGTH - 1] = '\0';
    } else {
        ESP_LOGE(TAG, "IP missing in the data");
        return -1;
    }

    ESP_LOGI(TAG, "Parsed SSID: %s", SSID);
    ESP_LOGI(TAG, "Parsed Password: %s", PASSWORD);
    ESP_LOGI(TAG, "Parsed Port: %s", PORT);
    ESP_LOGI(TAG, "Parsed IP: %s", IP);

    return 0;
}

// BLE write handler
static int ble_write_handler(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg) {
    char received_data[256];

    if (ctxt->om->om_len >= sizeof(received_data)) {
        ESP_LOGE(TAG, "Received data too long");
        return -1;
    }
    memcpy(received_data, ctxt->om->om_data, ctxt->om->om_len);
    received_data[ctxt->om->om_len] = '\0';

    ESP_LOGI(TAG, "Raw data received: %s", received_data);

    if (parse_received_data(received_data) != 0) {
        ESP_LOGE(TAG, "Failed to parse received data");
        return -1;
    }

    ESP_LOGI(TAG, "Starting Wi-Fi with received credentials");
    wifi_init_sta();

    return 0;
}

// GATT service definition
const struct ble_gatt_svc_def gatt_svcs[] = {
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID16_DECLARE(0x180),
        .characteristics = (struct ble_gatt_chr_def[]) {
            {
                .uuid = BLE_UUID16_DECLARE(0xF00D),
                .flags = BLE_GATT_CHR_F_WRITE,
                .access_cb = ble_write_handler,
            },
            {0}
        },
    },
    {0}
};
