#include <stdio.h>
#include "freertos/FreeRTOS.h" // ESP32 runs on FreeRTOS
#include "esp_wifi.h"  // Wi-Fi driver
#include "esp_event.h" // Event loop (Wi-Fi + MQTT need it)
#include "esp_log.h" // Logging
#include "nvs_flash.h" // Wi-Fi stores credentials in NVS
#include "mqtt_client.h" // MQTT support

#define WIFI_SSID      "YOUR_WIFI_SSID"
#define WIFI_PASSWORD  "YOUR_WIFI_PASSWORD"

#define MQTT_BROKER    "mqtt://192.168.1.100" // IP address of the MQTT broker (which is on localhost)
#define MQTT_TOPIC     "fpga/score" // "destination" for the message

static const char *TAG = "SCORE_SENDER";

static esp_mqtt_client_handle_t mqtt_client;

// WiFi init
static void wifi_init(void) {
    // Questa funzione è da rifare @Fra, l'ho copiata solo come placeholder

    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASSWORD,
        },
    };

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    esp_wifi_start();
    esp_wifi_connect();
}

// MQTT event handler
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_mqtt_event_handle_t event = event_data;

    if (event_id == MQTT_EVENT_CONNECTED) {
        ESP_LOGI(TAG, "MQTT connected"); // Essentially a log message
    }
}

// MQTT init
static void mqtt_init(void) {
    esp_mqtt_client_config_t mqtt_cfg = {
        .uri = MQTT_BROKER,
    };

    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);

    esp_mqtt_client_start(mqtt_client);
}

// Publish score
static void publish_score(int value, const char *game, const char *player) {
    char payload[128];

    snprintf(payload, sizeof(payload),
        "{\"game\":\"%s\",\"player\":\"%s\",\"score\":%d}", // Message format
        game, player, value);

    esp_mqtt_client_publish(mqtt_client, MQTT_TOPIC, payload, 0, 1, 0); // Similar to a POST request

    ESP_LOGI(TAG, "Published: %s", payload);
}

// Main task example
void app_main(void) {
    nvs_flash_init();
    wifi_init();
    mqtt_init();

    int score = 100;
    publish_score(score, "Snake", "Leo");
}
