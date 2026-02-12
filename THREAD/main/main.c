#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"

#include "driver/gpio.h"

#define led1 12
#define led2 13
#define led3 14

static const char *TAG = "THREAD";

// Task ou THREAD piscar LED 1
void task1(void *pvParam)
{
    // Variável que cuidará de acender o led 1
    int led1_state = 1;
    while (true)
    {
        // seta com o nivel do led1_state a saída
        gpio_set_level(led1, led1_state);
        led1_state = !led1_state;

        ESP_LOGI(TAG, "Mudou o led1");
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

// Task ou THREAD piscar LED 2
void task2(void *pvParam)
{
    // Variável que cuidará de acender o led 2
    int led2_state = 1;
    while (true)
    {
        // seta com o nivel do led2_state a saída
        gpio_set_level(led2, led2_state);
        led2_state = !led2_state;

        ESP_LOGI(TAG, "Mudou o led2");
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

// Task ou THREAD piscar LED 3
void task3(void *pvParam)
{
    // Variável que cuidará de acender o led 3
    int led3_state = 1;
    while (true)
    {
        // seta com o nivel do led3_state a saída
        gpio_set_level(led3, led3_state);
        led3_state = !led3_state;

        ESP_LOGI(TAG, "Mudou o led3");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void app_main(void)
{
    // Configura o pino como GPIO
    esp_rom_gpio_pad_select_gpio(led1);
    // Configura o GPIO como saída OUTPUT
    gpio_set_direction(led1, GPIO_MODE_OUTPUT);

    // Configura o pino como GPIO
    esp_rom_gpio_pad_select_gpio(led2);
    // Configura o GPIO como saída OUTPUT
    gpio_set_direction(led2, GPIO_MODE_OUTPUT);

    // Configura o pino como GPIO
    esp_rom_gpio_pad_select_gpio(led3);
    // Configura o GPIO como saída OUTPUT
    gpio_set_direction(led3, GPIO_MODE_OUTPUT);

    // cria a task, ou seja uma thread, fixado no CORE 2 
    xTaskCreatePinnedToCore(&task1, "task1", 2048, NULL, 1, NULL, 0);

    // cria a task, ou seja uma thread, fixado no CORE 1 (Paralelismo)
    xTaskCreatePinnedToCore(&task2, "task2", 2048, NULL, 2, NULL, 1);

    // cria a task, ou seja uma thread, fixado no CORE 2 
    xTaskCreatePinnedToCore(&task3, "task3", 2048, NULL, 1, NULL, 0);
    
}