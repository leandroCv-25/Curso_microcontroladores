#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"

const gpio_num_t cButton = GPIO_NUM_4;

void app_main(void)
{

	esp_rom_gpio_pad_select_gpio(cButton);
	gpio_set_direction(cButton, GPIO_MODE_INPUT);
	// gpio_set_pull_mode(cButton, GPIO_PULLUP_ONLY);
	// gopio_set_pull_mode(cButton, GPIO_PULLDOWN_ONLY);

	// Criamos uma variável que armazena o tempo da ultima mudança
	unsigned long lastDebounceTime = 0;
	// Cria variável que armazena o ultimo estado
	int lastState = 0;

	while (1)
	{
		// get o nível da entrada
		int reading = gpio_get_level(cButton);

		// Compara se o lastState é diferente do estado que está lendo, se houve mudança
		if (reading != lastState)
		{
			// Mensagem de LOG para depurar o sistema
			ESP_LOGI("Botao", "Opa, algo mudou");

			// Atribuindo lastDebounceTime com o tempo da ultima mudança
			lastDebounceTime = esp_timer_get_time();
		}

		if (((esp_timer_get_time() - lastDebounceTime) > 4000000) && reading)
		{
			// Mensagem de LOG para depurar o sistema
			ESP_LOGI("Botao", "Apertou o suficiente");
			lastDebounceTime = esp_timer_get_time();
		}

		// Atribuindo lastState com o estado lido
		lastState = reading;

		// delay de 50 ms o microcontrolador fica em estado idle se não tiver outra task em andamento
		vTaskDelay(pdMS_TO_TICKS(50));
	}
}
