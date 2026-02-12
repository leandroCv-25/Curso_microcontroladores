#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"

#include "liquid_crystal.h"

void app_main(void)
{

	liquid_crystal_connection_t liquid_crystal_connection = {
		.enable = 15,
		.rs = 2,
	};

	liquid_crystal_connection._data_pins[0] = 13;
	liquid_crystal_connection._data_pins[1] = 12;
	liquid_crystal_connection._data_pins[2] = 14;
	liquid_crystal_connection._data_pins[3] = 27;

	liquid_crystal_t liquid_crystal = {
		.liquid_crystal_connection = liquid_crystal_connection,
		.liquid_crystal_kind_connection = LIQUID_CRYSTAL_FOUR_BITs,
		.cols = 16,
		.rows = 2,
		.charsize = LCD_5x8DOTS,
	};

	liquid_crystal_init(&liquid_crystal);

	vTaskDelay(pdMS_TO_TICKS(1000));

	liquid_crystal_cursor(&liquid_crystal);

	liquid_crystal_set_cursor(&liquid_crystal,0,0);

	liquid_crystal_set_cursor(&liquid_crystal,1,1);

	liquid_crystal_blink(&liquid_crystal);
	

	vTaskDelay(pdMS_TO_TICKS(1000));
	while (true)
	{
		ESP_LOGI("lcd","teste");
		liquid_crystal_print(&liquid_crystal, "TESTE");
		vTaskDelay(pdMS_TO_TICKS(10000));
		liquid_crystal_clear(&liquid_crystal);
	}
}
