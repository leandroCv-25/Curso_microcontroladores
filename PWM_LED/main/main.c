#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"

#include "driver/gpio.h"
#include "driver/ledc.h"

static const char TAG[] = "PWM LED";

ledc_timer_t ledc_timer = LEDC_TIMER_0;
ledc_mode_t ledc_mode = LEDC_LOW_SPEED_MODE;		// Dependendo MCU pode ser HIGH SPEED MODE
ledc_timer_bit_t ledc_duty_res = LEDC_TIMER_8_BIT; // Resolução de 8 bit 2^8 // Alguns MCU podem ter problemas quando posto no 100% com a resolução máxima
uint32_t ledc_frequency = 4000;						// Frequencia PWM 4000Hz pode ser alterado
gpio_num_t led_gpio = GPIO_NUM_13;
ledc_channel_t ledc_channel = LEDC_CHANNEL_0;

void app_main(void)
{

	// Cria a variável de configuração do timer do PWM
	ledc_timer_config_t ledc_timer_congif = {
		.speed_mode = ledc_mode,
		.duty_resolution = ledc_duty_res,
		.timer_num = ledc_timer,
		.freq_hz = ledc_frequency,
		.clk_cfg = LEDC_AUTO_CLK};
	// Timer configurado
	ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer_congif));

	// Cria a variável de configuração do canal do PWM
	ledc_channel_config_t ledc_chan_config = {
		.speed_mode = ledc_mode,
		.channel = ledc_channel,
		.timer_sel = ledc_timer,
		.intr_type = LEDC_INTR_DISABLE, // Desabilita a interrupão canal de PWM
		.gpio_num = led_gpio,
		.duty = 0, // Set duty to 0%
		.hpoint = 0};
	ESP_ERROR_CHECK(ledc_channel_config(&ledc_chan_config));

	while (true)
	{
		// vai incrementar Duty até chegar ao máximo
		for (int duty = 0; duty <= 255; duty++)
		{

			ESP_LOGI(TAG, "Valor de duty cycle é %i", duty);
			// seta o valor duty
			ledc_set_duty(ledc_mode, ledc_channel, duty);
			// habilita a alteração
			ledc_update_duty(ledc_mode, ledc_channel);
			// 50ms
			vTaskDelay(pdMS_TO_TICKS(50));
		}

		// vai incrementar Duty até chegar ao máximo
		for (int duty = 255; duty > 0; duty--)
		{

			ESP_LOGI(TAG, "Valor de duty cycle é %i", duty);
			// seta o valor duty
			ledc_set_duty(ledc_mode, ledc_channel, duty);
			// habilita a alteração
			ledc_update_duty(ledc_mode, ledc_channel);
			// 50ms
			vTaskDelay(pdMS_TO_TICKS(50));
		}
	}
}
