#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_timer.h"

#include "driver/gpio.h"

#include "stepper_motor_drive.h"

static const char *TAG = "Motor Step";
int clock = 0;

static void stepper_timer_cb(void *pvParameters)
{
    stepper_motor_driver_context_t *motor_ctrl_ctx = (stepper_motor_driver_context_t *)pvParameters;

    if (clock >= (4095 - motor_ctrl_ctx->speed))
    {
        motor_ctrl_ctx->step_gpio_state = !motor_ctrl_ctx->step_gpio_state;
        gpio_set_level(motor_ctrl_ctx->step_gpio, motor_ctrl_ctx->step_gpio_state);
        clock = 0;
    }
    else
    {
        clock++;
    }
}

void set_stepper_motor_direction(stepper_motor_driver_context_t *motor_ctrl_ctx, bool new_direction)
{
    gpio_set_level(motor_ctrl_ctx->diretion_gpio, new_direction);
}

void set_stepper_motor_speed(stepper_motor_driver_context_t *motor_ctrl_ctx, int new_speed)
{
    motor_ctrl_ctx->speed = new_speed;
}

void stepper_motor_driver_config(stepper_motor_driver_context_t *motor_ctrl_ctx)
{

    motor_ctrl_ctx->step_gpio_state = 0;
    motor_ctrl_ctx->speed = 0;

    ESP_LOGI(TAG, "Criando o STEP motor");
    esp_rom_gpio_pad_select_gpio(motor_ctrl_ctx->step_gpio);
    gpio_set_direction(motor_ctrl_ctx->step_gpio, GPIO_MODE_OUTPUT);
    esp_rom_gpio_pad_select_gpio(motor_ctrl_ctx->diretion_gpio);
    gpio_set_direction(motor_ctrl_ctx->diretion_gpio, GPIO_MODE_OUTPUT);

    ESP_LOGI(TAG, "Criando o timer para fazer os pulsos do motor");
    const esp_timer_create_args_t periodic_timer_args = {
        .callback = stepper_timer_cb,
        .arg = motor_ctrl_ctx,
        .name = "stepper_loop"};
    esp_timer_handle_t stepper_loop_timer = NULL;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &stepper_loop_timer));

    ESP_LOGI(TAG, "Start! o controle vai começar a calcular agora");
    ESP_ERROR_CHECK(esp_timer_start_periodic(stepper_loop_timer, 5));
}