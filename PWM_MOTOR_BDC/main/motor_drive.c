#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_timer.h"

#include "driver/pulse_cnt.h"
#include "bdc_motor.h"
#include "pid_ctrl.h"

#include "motor_drive.h"

static const char *TAG = "PWM MOTOR BDC";

// Função de CALLBACK do PID
static void pid_loop_cb(void *args)
{
    static int last_pulse_count = 0;
    motor_control_context_t *ctx = (motor_control_context_t *)args;
    pcnt_unit_handle_t pcnt_unit = ctx->pcnt_encoder;
    pid_ctrl_block_handle_t pid_ctrl = ctx->pid_ctrl;
    bdc_motor_handle_t motor = ctx->motor;
    int expect_speed = ctx->expect_speed;
    set_motor_direction_t direction = ctx->direction;

    // get the result from rotary encoder
    int cur_pulse_count = 0;
    ESP_ERROR_CHECK(pcnt_unit_get_count(pcnt_unit, &cur_pulse_count));

    int real_pulses = 0;
    real_pulses = cur_pulse_count - last_pulse_count;
    last_pulse_count = cur_pulse_count;
    ctx->report_pulses = real_pulses;

    float error = 0;
    float new_speed = 0;
    // calculate the speed error
    if (direction == MOTOR_FORWARD)
    {
        error = expect_speed - real_pulses;
    }
    else
    {
        error = expect_speed + real_pulses;
    }

    // set the new speed
    pid_compute(pid_ctrl, error, &new_speed);
    bdc_motor_set_speed(motor, (uint32_t)new_speed);
}

void motor_pid_update(motor_control_context_t *motor_ctrl_ctx, float kp, float kd, float ki)
{
    pid_ctrl_parameter_t pid_update_param = {
        .kp = kp,
        .ki = ki,
        .kd = kd,
        .cal_type = PID_CAL_TYPE_INCREMENTAL,
        .max_output = BDC_MCPWM_DUTY_TICK_MAX - 1,
        .min_output = 0,
        .max_integral = 1000,
        .min_integral = -1000,
    };

    ESP_ERROR_CHECK(pid_update_parameters(motor_ctrl_ctx->pid_ctrl, &pid_update_param));
}

void set_motor_speed(motor_control_context_t *motor_ctrl_ctx, int new_speed)
{
    motor_ctrl_ctx->expect_speed = (int)(new_speed*(motor_ctrl_ctx->pulses_per_rotation)/6000);
}

float get_motor_speed(motor_control_context_t *motor_ctrl_ctx)
{
   return (motor_ctrl_ctx->report_pulses)*6000/(motor_ctrl_ctx->pulses_per_rotation);
}

void set_motor_direction(motor_control_context_t *motor_ctrl_ctx, set_motor_direction_t direction)
{

    if (direction == MOTOR_FORWARD)
    {
        motor_ctrl_ctx->direction = MOTOR_FORWARD;
        bdc_motor_forward(motor_ctrl_ctx->motor);
    }
    else if (direction == MOTOR_REVERSE)
    {
        motor_ctrl_ctx->direction = MOTOR_REVERSE;
        bdc_motor_reverse(motor_ctrl_ctx->motor);
    }
    else
    {
        motor_brake(motor_ctrl_ctx);
    }
}

void motor_brake(motor_control_context_t *motor_ctrl_ctx)
{
    set_motor_speed(motor_ctrl_ctx, 0);
    bdc_motor_brake(motor_ctrl_ctx->motor);
}

void motor_drive_config(motor_control_context_t *motor_ctrl_ctx, int bdc_mcpwm_gpio_a, int bdc_mcpwm_gpio_b, int bdc_encoder_gpio_a, int bdc_encoder_gpio_b, int group_id)
{

    ESP_LOGI(TAG, "Criando o DC motor");
    bdc_motor_config_t motor_config = {
        .pwm_freq_hz = BDC_MCPWM_FREQ_HZ,
        .pwma_gpio_num = bdc_mcpwm_gpio_a, // Setando as portas da ponte H
        .pwmb_gpio_num = bdc_mcpwm_gpio_b,
    };
    bdc_motor_mcpwm_config_t mcpwm_config = {
        .group_id = group_id,                           // Setando o timer do MCPWM
        .resolution_hz = BDC_MCPWM_TIMER_RESOLUTION_HZ, // Colocando a resolução do Timer do motor
    };

    // Criando a variável de manipulação do motor
    bdc_motor_handle_t motor = NULL;
    // Atribuindo as configurações
    ESP_ERROR_CHECK(bdc_motor_new_mcpwm_device(&motor_config, &mcpwm_config, &motor));
    // Colocando nossa variável no contexto para podermos manipular o motor fora da função
    motor_ctrl_ctx->motor = motor;

    ESP_LOGI(TAG, "Criando o driver que cuida de decodificar o encoder");
    pcnt_unit_config_t unit_config = {
        .high_limit = BDC_ENCODER_PCNT_HIGH_LIMIT, // Setando os limites de contagem
        .low_limit = BDC_ENCODER_PCNT_LOW_LIMIT,
        .flags.accum_count = true, // habilitando a acumular a contagem
    };

    // Criando o manipulador
    pcnt_unit_handle_t pcnt_unit = NULL;
    // Atribuindo a configuração no manipulador
    ESP_ERROR_CHECK(pcnt_new_unit(&unit_config, &pcnt_unit));
    // criando um filtro
    pcnt_glitch_filter_config_t filter_config = {
        .max_glitch_ns = 1000,
    };
    // Atribuindo o filtro no manipulador
    ESP_ERROR_CHECK(pcnt_unit_set_glitch_filter(pcnt_unit, &filter_config));

    // Criando o canal a (Variável que vai armazenar as portas)
    pcnt_chan_config_t chan_a_config = {
        .edge_gpio_num = bdc_encoder_gpio_a,
        .level_gpio_num = bdc_encoder_gpio_b,
    };
    pcnt_channel_handle_t pcnt_chan_a = NULL;
    ESP_ERROR_CHECK(pcnt_new_channel(pcnt_unit, &chan_a_config, &pcnt_chan_a));

    // Criando o canal b (Variável que vai armazenar as portas)
    pcnt_chan_config_t chan_b_config = {
        .edge_gpio_num = bdc_encoder_gpio_b,
        .level_gpio_num = bdc_encoder_gpio_a,
    };
    pcnt_channel_handle_t pcnt_chan_b = NULL;
    ESP_ERROR_CHECK(pcnt_new_channel(pcnt_unit, &chan_b_config, &pcnt_chan_b));

    // Configuração as opções de leitura para a contagem
    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(pcnt_chan_a, PCNT_CHANNEL_EDGE_ACTION_DECREASE, PCNT_CHANNEL_EDGE_ACTION_INCREASE));
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(pcnt_chan_a, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE));
    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(pcnt_chan_b, PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_DECREASE));
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(pcnt_chan_b, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE));

    // Criando um ponte de reset no limite
    ESP_ERROR_CHECK(pcnt_unit_add_watch_point(pcnt_unit, BDC_ENCODER_PCNT_HIGH_LIMIT));
    ESP_ERROR_CHECK(pcnt_unit_add_watch_point(pcnt_unit, BDC_ENCODER_PCNT_LOW_LIMIT));

    // Habilitando
    ESP_ERROR_CHECK(pcnt_unit_enable(pcnt_unit));
    // Resetando a contagem para o inicio
    ESP_ERROR_CHECK(pcnt_unit_clear_count(pcnt_unit));

    // START
    ESP_ERROR_CHECK(pcnt_unit_start(pcnt_unit));

    // Colocando nossa variável no contexto para podermos manipular o enconder fora da função
    motor_ctrl_ctx->pcnt_encoder = pcnt_unit;

    ESP_LOGI(TAG, "Criando o bloco de controle PID");
    // Configuração do PID
    pid_ctrl_parameter_t pid_runtime_param = {
        .kp = 0.6,
        .ki = 0.4,
        .kd = 0.2,
        .cal_type = PID_CAL_TYPE_INCREMENTAL, // Tipo de controle -> para o motor o ideal é incremental mesmo (Pensado na velocidade)
        .max_output = BDC_MCPWM_DUTY_TICK_MAX - 1,
        .min_output = 0,
        .max_integral = 1000,
        .min_integral = -1000,
    };

    // Criando a variável de manipulação do PID
    pid_ctrl_block_handle_t pid_ctrl = NULL;
    // Criando a variável de configuração
    pid_ctrl_config_t pid_config = {
        .init_param = pid_runtime_param,
    };
    // Atribuindo a configuração de controle
    ESP_ERROR_CHECK(pid_new_control_block(&pid_config, &pid_ctrl));
    // Colocando nossa variável no contexto para podermos manipular o controle pid fora da função
    motor_ctrl_ctx->pid_ctrl = pid_ctrl;

    ESP_LOGI(TAG, "Criando o timer para fazer o calculo do PID periodicamente");
    const esp_timer_create_args_t periodic_timer_args = {
        .callback = pid_loop_cb,
        .arg = motor_ctrl_ctx,
        .name = "pid_loop"};
    esp_timer_handle_t pid_loop_timer = NULL;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &pid_loop_timer));

    ESP_LOGI(TAG, "Habilitando o motor");
    ESP_ERROR_CHECK(bdc_motor_enable(motor));

    if (motor_ctrl_ctx->direction == MOTOR_REVERSE)
    {
        ESP_LOGI(TAG, "Motor girando sentido anti-horario");
        ESP_ERROR_CHECK(bdc_motor_reverse(motor));
    }
    else
    {
        motor_ctrl_ctx->direction = MOTOR_FORWARD;
        ESP_LOGI(TAG, "Motor girando sentido horario");
        ESP_ERROR_CHECK(bdc_motor_forward(motor));
    }

    ESP_LOGI(TAG, "Start! o controle vai começar a calcular agora");
    ESP_ERROR_CHECK(esp_timer_start_periodic(pid_loop_timer, BDC_PID_LOOP_PERIOD_MS * 1000));
}
