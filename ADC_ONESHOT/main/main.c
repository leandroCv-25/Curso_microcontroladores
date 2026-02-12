#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"

#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

const static char *TAG = "ADC";

static bool adc_calibration_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t *out_handle);
static void adc_calibration_deinit(adc_cali_handle_t handle);

void app_main(void)
{
    // Cria a variável que é utilizada para gerenciar a unidade adc no caso one shot
    adc_oneshot_unit_handle_t adc1_handle;
    // Cria a variável que é utilizada para armazenar a configuração da unidade adc no caso one shot
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1, // Pode ser Unidade 1 ou 2 depende do uC
        //.ulp_mode = ADC_ULP_MODE_DISABLE, Quando estiver usando Unidade 2
    };

    // Termina de assiociar as duas variáveis para configurar a unidade
    // ESP_ERROR_CHECK -> verifica se ouve algum erro
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

    // Configuração de leitura do ADC
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_12, // Resolução
        .atten = ADC_ATTEN_DB_11,    // Atenuador -> define o faixa de tensão que podemos ler 0 a 3,3V no caso, existe outras opções
    };

    // Configura um pino para a leitura no caso o canal 2 adc da unidade 1
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_0, &config));

    // Calibração
    adc_cali_handle_t adc1_calibration_channel_2_handle = NULL;

    bool channel2IsCalibrated = adc_calibration_init(ADC_UNIT_1, ADC_CHANNEL_0, ADC_ATTEN_DB_11, &adc1_calibration_channel_2_handle);

    //Variáveis de leitura
    int adc_raw;
    int voltage;

    while (1)
    {
        //Faz a leitura do ADC_CHANNEL_0 e passa o dados para a variável adc_raw
        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC_CHANNEL_0, &adc_raw));
        //Podemos observar os resultados no monitor -> Ctrl+E e M
        ESP_LOGI(TAG, "ADC1 Channel[%d] -> valor lido em bits: %d", ADC_CHANNEL_0, adc_raw);
        if (channel2IsCalibrated) //Se o canal foi calibrado poderemos ver os dados em miliVolts
        {
            //Fazendo a conversão e colocando na variável voltage
            ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc1_calibration_channel_2_handle, adc_raw, &voltage));
            //Podemos observar os resultados no monitor -> Ctrl+E e M
            ESP_LOGI(TAG, "ADC1 Channel[%d] -> valor lido com em tensão: %d mV", ADC_CHANNEL_0, voltage);
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    //Podemos remover a calibração para utilizar de outra forma.
    adc_calibration_deinit(adc1_calibration_channel_2_handle);
}

/*---------------------------------------------------------------
        Calibração ADC
---------------------------------------------------------------*/
static bool adc_calibration_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t *out_handle)
{
    adc_cali_handle_t handle = NULL;
    esp_err_t ret = ESP_FAIL;
    bool calibrated = false;

// Calibração baseada no ajuste de curva.... Depende do MCU que está usando
// Exemplo https://www.researchgate.net/publication/334206969/figure/fig1/AS:776639436845056@1562176679172/Figura-41-Ajuste-de-curvas-por-Regressao.ppm
#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    if (!calibrated)
    {
        ESP_LOGI(TAG, "O esquema de calibração é %s", "ajuste de curva");
        adc_cali_curve_fitting_config_t cali_config = {
            .unit_id = unit,
            .chan = channel,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_curve_fitting(&cali_config, &handle);
        if (ret == ESP_OK)
        {
            calibrated = true;
        }
    }
#endif

// Calibração baseada no ajuste linear.... Depende do MCU que está usando
#if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    if (!calibrated)
    {
        ESP_LOGI(TAG, "O esquema de calibração é %s", "ajuste linear");
        adc_cali_line_fitting_config_t cali_config = {
            .unit_id = unit,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_line_fitting(&cali_config, &handle);
        if (ret == ESP_OK)
        {
            calibrated = true;
        }
    }
#endif

    *out_handle = handle;
    if (ret == ESP_OK)
    {
        ESP_LOGI(TAG, "Sucesso");
    }
    else if (ret == ESP_ERR_NOT_SUPPORTED || !calibrated)
    {
        ESP_LOGW(TAG, "eFuse não apropriado, pulando a calibração");
    }
    else
    {
        ESP_LOGE(TAG, "ERRO");
    }

    return calibrated;
}


/*---------------------------------------------------------------
        Desativa Calibração ADC
---------------------------------------------------------------*/
static void adc_calibration_deinit(adc_cali_handle_t handle)
{
    ESP_ERROR_CHECK(adc_cali_delete_scheme_line_fitting(handle));
}