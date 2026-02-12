#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "esp_log.h"

#include "driver/gpio.h"

#define RESET_BUTTON 4

static const char TAG[] = "INTERRUPCAO";

// gerenciador do semaforo do timer
SemaphoreHandle_t reset_semaphore = NULL;

void IRAM_ATTR reset_button_isr_handler(void *arg)
{
    // Notify the button task
    xSemaphoreGiveFromISR(reset_semaphore, NULL);
}

void app_main(void)
{

    // Cria o semaforo binário para notificar as trefas envolvidas
    reset_semaphore = xSemaphoreCreateBinary();

    // Configura o pino como GPIO
    esp_rom_gpio_pad_select_gpio(RESET_BUTTON);
    // Configura o GPIO como saída INPUT
    gpio_set_direction(RESET_BUTTON, GPIO_MODE_INPUT);

    // Habilita a interrupção no momento de descida do botão
    gpio_set_intr_type(RESET_BUTTON, GPIO_INTR_NEGEDGE);//

    // Install gpio isr service
    gpio_install_isr_service(0);

    // Anexa a interrupção a uma função
    gpio_isr_handler_add(RESET_BUTTON, reset_button_isr_handler, NULL);

    //Variável de contagem
    int count = 0;
    while (true)
    {   
        // Espera o Semaforo habilitar no máximo 1000ms ou se o contador chegar em 60
        if ((xSemaphoreTake(reset_semaphore, pdMS_TO_TICKS(1000)) == pdTRUE) || count == 60)
        {
            count = 0;
        }
        else
        {
            count++;
        }

        ESP_LOGI(TAG, "count %d", count);
    }
}
