#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/dac_oneshot.h"

void app_main(void)
{
    //Criando a variável que gerencia a DAC
    dac_oneshot_handle_t chan0_handle;

    //Criando a configuração
    dac_oneshot_config_t chan0_cfg = {
        .chan_id = DAC_CHAN_0, //Pino selecionado
    };

    //Anexando a configuração na nossa variável
    dac_oneshot_new_channel(&chan0_cfg, &chan0_handle);

    uint8_t value = 0;
    while (1) {
        /* Definindo o valor da tensão */
        dac_oneshot_output_voltage(chan0_handle, value);
        value += 10;
        value %= 250;
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    
}
