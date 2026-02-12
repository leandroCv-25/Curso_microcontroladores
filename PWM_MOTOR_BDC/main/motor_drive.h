#ifndef MOTOR_DRIVE_H_
#define MOTOR_DRIVE_H_

#include "driver/pulse_cnt.h"
#include "bdc_motor.h"
#include "pid_ctrl.h"

#define BDC_MCPWM_TIMER_RESOLUTION_HZ 10000000                                      // 10MHz, 1 tick = 0.1us
#define BDC_MCPWM_FREQ_HZ 25000                                                     // 25KHz PWM
#define BDC_MCPWM_DUTY_TICK_MAX (BDC_MCPWM_TIMER_RESOLUTION_HZ / BDC_MCPWM_FREQ_HZ) // maximum value we can set for the duty cycle, in ticks
#define BDC_PID_LOOP_PERIOD_MS 10                                                   // calculate the motor speed every 10ms
#define BDC_ENCODER_PCNT_HIGH_LIMIT 1000
#define BDC_ENCODER_PCNT_LOW_LIMIT -1000

typedef enum {
    MOTOR_FORWARD,
    MOTOR_REVERSE,
} set_motor_direction_t;

/**
 * @brief Estrutura para representar o contexto do motor
 * Sua variável que auxilia a gerenciar o motor
 * Sua variável que auxilia a gerenciar o encoder
 * Sua variável que auxilia a gerenciar o PID
 * 
 */
typedef struct
{
    bdc_motor_handle_t motor;
    pcnt_unit_handle_t pcnt_encoder;
    pid_ctrl_block_handle_t pid_ctrl;
    int report_pulses;
    int expect_speed;
    int pulses_per_rotation;
    set_motor_direction_t direction;
} motor_control_context_t;



/**
 * @brief Função para a atualização do Controle PID
 * 
 * @param motor_ctrl_ctx 
 * @param kp 
 * @param kd 
 * @param kp 
 */
void motor_pid_update(motor_control_context_t *motor_ctrl_ctx, float kp, float kd, float ki);

/**
 * @brief Seta a velocidade esperada do motor
 * 
 * @param motor_ctrl_ctx Contexto de controle de motor
 * @param new_speed Em RPM
 */
void set_motor_speed(motor_control_context_t *motor_ctrl_ctx, int new_speed);

/**
 * @brief Seta a velocidade esperada do motor
 * 
 * @param motor_ctrl_ctx Contexto de controle de motor
 * @param new_speed Em RPM
 */

/**
 * @brief Obtem a velocidade do motor em rpm
 * 
 * @param motor_ctrl_ctx Contexto de controle de motor
 * @return float 
 */
float get_motor_speed(motor_control_context_t *motor_ctrl_ctx);

/**
 * @brief Seta a direção do motor
 * 
 * @param motor_ctrl_ctx Contexto de controle de motor
 * @param direction 
 */
void set_motor_direction(motor_control_context_t *motor_ctrl_ctx, set_motor_direction_t direction);

/**
 * @brief Freia o motor e seta a velocidade em zero
 * 
 * @param motor_ctrl_ctx Contexto de controle de motor
 */
void motor_brake(motor_control_context_t *motor_ctrl_ctx);

/**
 * @brief Função para a configuração do motor BCD com controle de velocidade com PID
 * 
 * @param motor_ctrl_ctx Contexto do motor
 * @param bdc_mcpwm_gpio_a  GPIO ponte H - A
 * @param bdc_mcpwm_gpio_b  GPIO ponte H - B
 * @param bdc_encoder_gpio_a  GPIO enconder - A
 * @param bdc_encoder_gpio_b GPIO enconder - B
 * @param group_id Timer utilizado para MCPWM (A quantidade depende do MCU -> ESP32 tem dois)
 */
void motor_drive_config(motor_control_context_t *motor_ctrl_ctx, int bdc_mcpwm_gpio_a, int bdc_mcpwm_gpio_b, int bdc_encoder_gpio_a, int bdc_encoder_gpio_b, int group_id);

#endif