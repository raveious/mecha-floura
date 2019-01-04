#include "freertos/FreeRTOS.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include <uxr/client/client.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

esp_err_t event_handler(void *ctx, system_event_t *event)
{
    return ESP_OK;
}

bool on_agent_found(const uxrAgentAddress* address, int64_t timestamp, void* args) {
    (void) timestamp; (void) args;

    printf("Found agent => ip: %s, port: %d\n", address->ip, address->port);

    return false;
}

void app_main(void)
{
    nvs_flash_init();
    tcpip_adapter_init();
    ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    wifi_config_t sta_config = {
        .sta = {
            .ssid = CONFIG_ESP_WIFI_SSID,
            .password = CONFIG_ESP_WIFI_PASSWORD,
            .bssid_set = false
        }
    };
    ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &sta_config) );
    ESP_ERROR_CHECK( esp_wifi_start() );
    ESP_ERROR_CHECK( esp_wifi_connect() );

    uxrAgentAddress chosen;

    if(uxr_discovery_agents_multicast(10, 1000, on_agent_found, NULL, &chosen)) {
        // True -> The user returns true in the callback.
        printf("Chosen agent => ip: %s, port: %d\n", chosen.ip, chosen.port);
    }

    fflush(stdout);

    gpio_set_direction(GPIO_NUM_4, GPIO_MODE_OUTPUT);
    int level = 0;
    while (true) {
        gpio_set_level(GPIO_NUM_4, level);
        level = !level;
        vTaskDelay(300 / portTICK_PERIOD_MS);
    }
}

