#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"

#include "motor_drive.h"

static const char *TAG = "Main - PWM MOTOR BDC";

void app_main(void)
{
    static motor_control_context_t motor_ctrl_ctx = {
        .pcnt_encoder = NULL,
        .expect_speed = 0,
        .pulses_per_rotation = 408,
        .direction = MOTOR_FORWARD};

    motor_drive_config(&motor_ctrl_ctx, 18, 19, 33, 32, 0);

    vTaskDelay(pdMS_TO_TICKS(1000));

    while (true)
    {
        for (int i=0; i < 150; i++)
        {
            ESP_LOGI(TAG, "Setando velocidade %d e Sentido horario", i);
            set_motor_direction(&motor_ctrl_ctx, MOTOR_FORWARD);
            set_motor_speed(&motor_ctrl_ctx, i);
            vTaskDelay(pdMS_TO_TICKS(100));
            ESP_LOGI(TAG, "Velocidade %.2f RPM", get_motor_speed(&motor_ctrl_ctx));
        }

        ESP_LOGI(TAG, "FREIANDO");
        motor_brake(&motor_ctrl_ctx);
        vTaskDelay(pdMS_TO_TICKS(100));

        for (int i=0; i < 150; i++)
        {
            ESP_LOGI(TAG, "Setando velocidade %d e Sentido anti-horario", i);
            set_motor_direction(&motor_ctrl_ctx, MOTOR_REVERSE);
            set_motor_speed(&motor_ctrl_ctx, i);
            vTaskDelay(pdMS_TO_TICKS(100));
            ESP_LOGI(TAG, "Velocidade %.2f RPM", get_motor_speed(&motor_ctrl_ctx));
        }
    
        ESP_LOGI(TAG, "FREIANDO");
        motor_brake(&motor_ctrl_ctx);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}