#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"

#include "driver/gpio.h"

#include "stepper_motor_drive.h"

#include "esp_adc/adc_oneshot.h"

void app_main(void)
{
    int input_direction = 25;

    // Cria a variável que é utilizada para gerenciar a unidade adc no caso one shot
    adc_oneshot_unit_handle_t adc1_handle;

    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1, // Pode ser Unidade 1 ou 2 depende do uC
        //.ulp_mode = ADC_ULP_MODE_DISABLE, Quando estiver usando Unidade 2
    };

    // Termina de assiociar as duas variáveis para configurar a unidade
    // ESP_ERROR_CHECK -> verifica se ouve algum erro
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

    // Configuração de leitura do ADC
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_12, // Resolução
        .atten = ADC_ATTEN_DB_12,    // Atenuador -> define o faixa de tensão que podemos ler 0 a 3,3V no caso, existe outras opções
    };

    // Configura um pino para a leitura no caso o canal 2 adc da unidade 1
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_6, &config));

    esp_rom_gpio_pad_select_gpio(input_direction);
    gpio_set_direction(input_direction, GPIO_MODE_INPUT);

    stepper_motor_driver_context_t motor_ctrl_ctx = {
        .step_gpio = 18,
        .diretion_gpio = 19,
    };

    int adc_raw;

    stepper_motor_driver_config(&motor_ctrl_ctx);
    while (1)
    {

        set_stepper_motor_direction(&motor_ctrl_ctx, gpio_get_level(input_direction));
        adc_oneshot_read(adc1_handle, ADC_CHANNEL_6, &adc_raw);

        set_stepper_motor_speed(&motor_ctrl_ctx, adc_raw);

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
