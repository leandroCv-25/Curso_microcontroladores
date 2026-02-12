#include <stdio.h>

//Devemos incluir as bibliotecas do FreeRTOS e do driver GPIO para poder usar as funções de controle de tarefas e espera (delay).
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


//Incluímos a biblioteca do driver GPIO para poder configurar os pinos de saída e controlar o LED.
#include "driver/gpio.h"

gpio_num_t light = GPIO_NUM_2;

void app_main(void)
{
	/*
  	A primeira coisa que devemos fazer aqui no app_main é configurar as periféricos que vamos usar. No nosso caso, vamos configurar o GPIO para controlar um LED.
  	*/

	// Configura o pino como GPIO
	esp_rom_gpio_pad_select_gpio(light);
	// Configura o GPIO como saída OUTPUT
	gpio_set_direction(light, GPIO_MODE_OUTPUT);

	while (true)
	{
		// seta como nível 1 ou true a saída
		gpio_set_level(light, 1);
		// delay de 2000 ms o microcontrolador fica em estado idle se não tiver outra task em andamento
		vTaskDelay(pdMS_TO_TICKS(2000));
		// seta como nível 0 ou false a saída
		gpio_set_level(light, 0);
		// delay de 2000 ms o microcontrolador fica em estado idle se não tiver outra task em andamento
		vTaskDelay(pdMS_TO_TICKS(2000));
	}
}
