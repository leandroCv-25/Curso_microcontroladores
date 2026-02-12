#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"

#define cButton 4

void app_main(void)
{

	// Configura o pino como GPIO
	esp_rom_gpio_pad_select_gpio(cButton);
	// Configura o GPIO como saída INPUT
	gpio_set_direction(cButton, GPIO_MODE_INPUT);

	//Cria variável que armazena o tempo da ultima mudança
	unsigned long lastDebounceTime = 0;
	//Cria variável que armazena o ultimo estado
	int lastState = 0;
	while (true)
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
		// seta como nível 0 ou LOW a saída
	}
}
