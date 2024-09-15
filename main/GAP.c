#include "GAP.h"
#include "esp_log.h"
#include "host/ble_hs.h"

static const char *TAG = "BLE_GAP";

// GAP event handler
int ble_gap_event(struct ble_gap_event *event, void *arg) {
    switch (event->type) {
        case BLE_GAP_EVENT_CONNECT:
            ESP_LOGI(TAG, "BLE GAP EVENT CONNECT %s", event->connect.status == 0 ? "OK!" : "FAILED!");
            if (event->connect.status != 0) {
                ble_app_advertise();
            }
            break;
        case BLE_GAP_EVENT_DISCONNECT:
            ESP_LOGI(TAG, "BLE GAP EVENT DISCONNECTED");
            ble_app_advertise();
            break;
        case BLE_GAP_EVENT_ADV_COMPLETE:
            ESP_LOGI(TAG, "BLE GAP EVENT ADV COMPLETE");
            ble_app_advertise();
            break;
        default:
            break;
    }
    return 0;
}

// Start advertising
void ble_app_advertise(void) {
    struct ble_hs_adv_fields fields;
    const char *device_name = ble_svc_gap_device_name();
    
    memset(&fields, 0, sizeof(fields));
    fields.name = (uint8_t *)device_name;
    fields.name_len = strlen(device_name);
    ble_gap_adv_set_fields(&fields);

    struct ble_gap_adv_params adv_params = {
        .conn_mode = BLE_GAP_CONN_MODE_UND,
        .disc_mode = BLE_GAP_DISC_MODE_GEN,
    };

    ble_gap_adv_start(ble_addr_type, NULL, BLE_HS_FOREVER, &adv_params, ble_gap_event, NULL);
}

void ble_app_on_sync(void) {
    ble_hs_id_infer_auto(0, &ble_addr_type);
    ble_app_advertise();
}

void host_task(void *param) {
    nimble_port_run();
}

void Ble_task(void *pvParameters) {
    nimble_port_init();
    ble_svc_gap_device_name_set("BleServer");
    ble_svc_gap_init();
    ble_svc_gatt_init();
    ble_gatts_count_cfg(gatt_svcs);
    ble_gatts_add_svcs(gatt_svcs);
    ble_hs_cfg.sync_cb = ble_app_on_sync;
    nimble_port_freertos_init(host_task);

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
