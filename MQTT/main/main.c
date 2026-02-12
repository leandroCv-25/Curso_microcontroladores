#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include <nvs_flash.h>

#include "driver/gpio.h"

#include "iot_button.h"
#include "mqtt_client.h"

#include "button_app.h"
#include "wifi_app.h"
#include "mqtt_app.h"

static const char TAG[] = "MAIN MQTT";

esp_mqtt_client_handle_t client;

int light_state = 0;

static void wifi_conection(void *arg, void *usr_data)
{
	// enviando na fila a ordem de iniciar o smart config
	wifi_app_send_message(WIFI_APP_MSG_USER_REQUESTED_CONNECTION);
}

static void mqtt_recevied_msg(int msg)
{
	light_state = msg;
}

static void msg_switch(void *arg, void *usr_data)
{

	int *light_state = (int *)usr_data;
	int msg = !*light_state;

	mqtt_app_send_msg(client, msg);
}

void wifi_application_connected_events(void)
{
	ESP_LOGI(TAG, "WiFi Application Connected!!");
	client = mqtt_app_start(&mqtt_recevied_msg);
}

void app_main(void)
{
	/* Inicializando o NVS
	Nossa memoria flash
	 */
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
	{
		/* NVS partition was truncated
		 * and needs to be erased */
		ESP_ERROR_CHECK(nvs_flash_erase());

		/* Retry nvs_flash_init */
		ESP_ERROR_CHECK(nvs_flash_init());
	}

	gpio_num_t light = 2;
	// Configura o pino como GPIO
	esp_rom_gpio_pad_select_gpio(light);
	// Configura o GPIO como saída OUTPUT
	gpio_set_direction(light, GPIO_MODE_OUTPUT);
	light_state = 0;

	ESP_LOGI(TAG, "Iniciando WI-FI APP");
	wifi_app_start(&wifi_application_connected_events);

	ESP_LOGI(TAG, "Iniciando o botão");
	button_handle_t gpio_btn = button_app_init(4, 1);

	resgister_event_callback(gpio_btn, BUTTON_SINGLE_CLICK, 2000, msg_switch, &light_state);
	resgister_event_callback(gpio_btn, BUTTON_LONG_PRESS_START, 8000, wifi_conection, NULL);

	while (true)
	{
		gpio_set_level(light, light_state);
		vTaskDelay(pdMS_TO_TICKS(50));
	}
}
