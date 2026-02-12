#ifndef STEPPER_MOTOR_DRIVE_H_
#define STEPPER_MOTOR_DRIVE_H_

/**
 * @brief Estrutura para representar o contexto do motor
 *
 */
typedef struct stepper_motor_driver_context
{
    int step_gpio;
    int diretion_gpio;
    int speed;
    bool step_gpio_state;
} stepper_motor_driver_context_t;

/**
 * @brief Seta a velocidade esperada do motor
 *
 * @param motor_ctrl_ctx Contexto de controle de motor
 * @param new_speed mm/s
 */
void set_stepper_motor_speed(stepper_motor_driver_context_t *motor_ctrl_ctx, int new_speed);

/**
 * @brief Set the stepper motor direction
 * 
 * @param motor_ctrl_ctx 
 * @param new_direction 
 */
void set_stepper_motor_direction(stepper_motor_driver_context_t *motor_ctrl_ctx, bool new_direction);

/**
 * @brief Função para a configuração do motor passo
 *
 */
void stepper_motor_driver_config(stepper_motor_driver_context_t *motor_ctrl_ctx);

#endif