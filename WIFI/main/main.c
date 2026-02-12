#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include <nvs_flash.h>

#include "iot_button.h"

#include "wifi_app.h"

static const char TAG[] = "MAIN WIFI";

static void wifi_conection(void *arg, void *usr_data)
{	
	//enviando na fila a ordem de iniciar o smart config
	wifi_app_send_message(WIFI_APP_MSG_USER_REQUESTED_CONNECTION);
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

	ESP_LOGI(TAG, "Iniciando WI-FI APP");
	wifi_app_start();

	ESP_LOGI(TAG, "Iniciando o botão");

	// criando o botão na GPIO 4 e com tipo de digitais
	button_config_t gpio_btn_cfg = {
		.type = BUTTON_TYPE_GPIO,
		.long_press_time = CONFIG_BUTTON_LONG_PRESS_TIME_MS, //Tempo para considerar apertado por longo periodo
		.short_press_time = CONFIG_BUTTON_SHORT_PRESS_TIME_MS, //tempo para considerar apertado por curto periodo
		.gpio_button_config = {
			.gpio_num = 4,
			.active_level = 1, //Nível que considerar apertado
		},
	};

	//Variável de gerenciamento do botão sendo configurada
	button_handle_t gpio_btn = iot_button_create(&gpio_btn_cfg);
	if (NULL == gpio_btn)
	{
		ESP_LOGE(TAG, "Button create failed");
	}

	button_event_config_t cfg = {
		.event = BUTTON_LONG_PRESS_START, //Evento pressionado por um longo periodo
		.event_data.long_press.press_time = 8000, //quanto tempo para considerar longo periodo e chamar o evento de smart config
	};

	//Registrando evento com função criada
	iot_button_register_event_cb(gpio_btn, cfg, wifi_conection, NULL);
}
