
#include "esp_check.h"

#include "driver/gpio.h"

#include "iot_button.h"

#include "button_app.h"


esp_err_t resgister_event_callback(button_handle_t gpio_btn, button_event_t button_event, uint16_t time_pressed, button_cb_t cb, void *usr_data)
{
    button_event_config_t cfg = {
        .event = button_event,                            // Evento pressionado por um longo periodo
        .event_data.long_press.press_time = time_pressed, // quanto tempo para considerar longo periodo e chamar o evento de smart config
    };

    // Registrando evento com função criada
    return iot_button_register_event_cb(gpio_btn, cfg, cb, usr_data);
}

button_handle_t button_app_init(gpio_num_t gpio_btn, int active_level)
{

    // criando o botão na GPIO gpio_btn e com tipo de digitais
    button_config_t gpio_btn_cfg = {
        .type = BUTTON_TYPE_GPIO,
        .long_press_time = CONFIG_BUTTON_LONG_PRESS_TIME_MS,   // Tempo para considerar apertado por longo periodo
        .short_press_time = CONFIG_BUTTON_SHORT_PRESS_TIME_MS, // tempo para considerar apertado por curto periodo
        .gpio_button_config = {
            .gpio_num = gpio_btn,
            .active_level = active_level, // Nível que considerar apertado
        },
    };

    // Variável de gerenciamento do botão sendo configurada
    button_handle_t gpio_btn_handle = iot_button_create(&gpio_btn_cfg);

    return gpio_btn_handle;
}