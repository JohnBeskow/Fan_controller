#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_adc/adc_oneshot.h"
#include "driver/gpio.h"
#include "GATT.h"
#include "GAP.h"
#include "lwip/sockets.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

// Tag for logging
static const char *TAG = "esp32_John";
static const char *TAGTCPCLI = "TCP_client";

// Event group for Wi-Fi connection
static EventGroupHandle_t wifi_event_group;
const int CONNECTED_BIT = BIT0;

// Fan GPIO settings
#define FAN_GPIO GPIO_NUM_23
bool level = 0; // Fan default state

// ADC for the temperature sensor
#define TMP_SENSOR_PIN ADC_CHANNEL_3  // Corresponds to GPIO3 (ADC1_CHANNEL_3)
static adc_oneshot_unit_handle_t adc1_handle;

// Configure ADC
void configure_adc() {
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
    };
    adc_oneshot_new_unit(&init_config, &adc1_handle);
    
    adc_oneshot_chan_cfg_t config = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_12,
    };
    adc_oneshot_config_channel(adc1_handle, TMP_SENSOR_PIN, &config);
}

// Wi-Fi event handler
static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));

        ESP_LOGI(TAG, "Starting TCP client...");
        xTaskCreate(tcpclient, "tcp_client", 10096, NULL, 5, NULL);
    }
}

// Initialize Wi-Fi
void wifi_init_sta(void) {
    esp_netif_init();
    wifi_event_group = xEventGroupCreate();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, &instance_any_id);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, &instance_got_ip);

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = {0},
            .password = {0},
        },
    };
    strncpy((char*)wifi_config.sta.ssid, SSID, sizeof(wifi_config.sta.ssid));
    strncpy((char*)wifi_config.sta.password, PASSWORD, sizeof(wifi_config.sta.password));

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);
    esp_wifi_start();

    ESP_LOGI(TAG, "Wi-Fi reinitialized with new credentials.");
}

void app_main(void) {
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize ADC
    configure_adc();

    // Start BLE task
    xTaskCreate(Ble_task, "Ble_task", 8192, NULL, 5, NULL);
}
