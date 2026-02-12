#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "esp_log.h"

#include "driver/gptimer.h"
#include "driver/gpio.h"

#define led1 12
#define led2 13

static const char *TAG = "TIMER";

// gerenciador do semaforo do timer
SemaphoreHandle_t timer_semaphore = NULL;

static bool IRAM_ATTR timer_on_alarm(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *pvParam)
{
    BaseType_t high_task_awoken = pdFALSE;
    // Poderiamos parar o timer com essa linha
    // gptimer_stop(timer);

    // Notifica a task do timer
    xSemaphoreGiveFromISR(timer_semaphore, NULL);

    // Retornamos para a tarefa
    return (high_task_awoken == pdTRUE);
}

// Task dedicada para cuidar do timer
void timer_task(void *pvParam)
{

    // Variável que cuidará de acender o led 2
    int led2_state = 1;
    while (true)
    {
        // Espera o Semaforo habilitar para realizar a ação (esperando o tempo máximo -> portMAX_DELAY)
        if (xSemaphoreTake(timer_semaphore, portMAX_DELAY) == pdTRUE)
        {
            // seta com o nivel do led2_state a saída
            gpio_set_level(led2, led2_state);
            led2_state = !led2_state;

            ESP_LOGI(TAG, "Mudou o led2");
            for (int i = 0; i < 10; i++)
                ESP_LOGI(TAG, ".");
        }
    }
}

void app_main(void)
{
    // Cria um semaforo binario
    timer_semaphore = xSemaphoreCreateBinary();

    ESP_LOGI(TAG, "Gerenciador do timer criado");
    gptimer_handle_t gptimer = NULL;
    // configurando  timer
    gptimer_config_t timer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT, // Entrada do CLOCK
        .direction = GPTIMER_COUNT_UP,      // contagem subindo
        .resolution_hz = 1000000,           // 1MHz, 1 tick=1us Frequencia do timer
    };
    // Termina a configuração
    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &gptimer));

    // Cria um alarm
    gptimer_event_callbacks_t cbs = {
        .on_alarm = timer_on_alarm,
    };
    // Registra o alarme no timer
    ESP_ERROR_CHECK(gptimer_register_event_callbacks(gptimer, &cbs, NULL));

    ESP_LOGI(TAG, "Habilita o timer");
    ESP_ERROR_CHECK(gptimer_enable(gptimer));

    ESP_LOGI(TAG, "Configuração o alarme para o acionamento");
    gptimer_alarm_config_t alarm_config1 = {
        .reload_count = 0,                  // Se tiver que fazer a contagem novamente em que ponto começa
        .alarm_count = 5000000,             // period = 5s até quando o timer tem que contar até dar o alarme
        .flags.auto_reload_on_alarm = true, // Aciona para começar
    };

    // Configurando o timer
    ESP_ERROR_CHECK(gptimer_set_alarm_action(gptimer, &alarm_config1));
    // Iniciando a contagem
    ESP_LOGI(TAG, "Start timer");
    ESP_ERROR_CHECK(gptimer_start(gptimer));

    // Configura o pino como GPIO
    esp_rom_gpio_pad_select_gpio(led1);
    // Configura o GPIO como saída OUTPUT
    gpio_set_direction(led1, GPIO_MODE_OUTPUT);

    // Configura o pino como GPIO
    esp_rom_gpio_pad_select_gpio(led2);
    // Configura o GPIO como saída OUTPUT
    gpio_set_direction(led2, GPIO_MODE_OUTPUT);

    // cria a task do timer fixado no CORE 1
    xTaskCreatePinnedToCore(&timer_task, "timer_task", 2048, NULL, 1, NULL, 1);

    // Variável que cuidará de acender o led 1
    int led1_state = 0;
    while (true)
    {
        // seta com o nivel do led1_state a saída
        gpio_set_level(led1, led1_state);
        vTaskDelay(pdMS_TO_TICKS(5000));
        led1_state = !led1_state;

        ESP_LOGI(TAG, "Mudou o led1");
        for (int i = 0; i < 10; i++)
            ESP_LOGI(TAG, ".");
    }

    // O programa não chega aqui por causa do Loop, mas se for necessário está aqui as funções
    ESP_LOGI(TAG, "Parar o timer");
    ESP_ERROR_CHECK(gptimer_stop(gptimer));
    ESP_LOGI(TAG, "Desabilitar o timer");
    ESP_ERROR_CHECK(gptimer_disable(gptimer));
    ESP_LOGI(TAG, "Deletar timer"); // Limpando a memoria
    ESP_ERROR_CHECK(gptimer_del_timer(gptimer));
}
