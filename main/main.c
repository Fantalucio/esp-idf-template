#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "driver/gpio.h"



#define BLINK_GPIO GPIO_NUM_4

void initIO(void);




/*

esp_err_t event_handler(void *ctx, system_event_t *event)
{
    return ESP_OK;
}
*/


// event group
static EventGroupHandle_t wifi_event_group;
const int CONNECTED_BIT = BIT0;

// Wifi event handler
static esp_err_t event_handler(void *ctx, system_event_t *event) {
	switch (event->event_id) {

	case SYSTEM_EVENT_STA_START:
		esp_wifi_connect();
		break;

	case SYSTEM_EVENT_STA_GOT_IP:
		xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
		break;

	case SYSTEM_EVENT_STA_DISCONNECTED:
		xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
		break;

	default:
		break;
	}

	return ESP_OK;
}



void blink_task(void *pvParameter) {
	/* Configure the IOMUX register for pad BLINK_GPIO (some pads are
	 muxed to GPIO on reset already, but some default to other
	 functions and need to be switched to GPIO. Consult the
	 Technical Reference for a list of pads and their default
	 functions.)
	 */
//	EventBits_t eventBits;
	gpio_pad_select_gpio(BLINK_GPIO);
	/* Set the GPIO as a push/pull output */
	gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
	while (1) {
		/* Blink off (output low) */
		gpio_set_level(BLINK_GPIO, 0);

//		eventBits = xEventGroupGetBits(wifi_event_group);

		if ((xEventGroupGetBits(wifi_event_group) & CONNECTED_BIT) == 0) {
			vTaskDelay(100 / portTICK_PERIOD_MS);
		} else {
			vTaskDelay(1000 / portTICK_PERIOD_MS);
		}

		/* Blink on (output high) */
		gpio_set_level(BLINK_GPIO, 1);
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
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

	initIO();
	// start the main task
	xTaskCreate(&blink_task, "blink_task", 1024, NULL, 5, NULL);


}


void initIO(void) {
	gpio_pad_select_gpio(GPIO_NUM_4);
	gpio_pad_select_gpio(GPIO_NUM_5);
	gpio_set_direction(GPIO_NUM_4, GPIO_MODE_DEF_OUTPUT);
	gpio_set_direction(GPIO_NUM_5, GPIO_MODE_DEF_OUTPUT);

}


