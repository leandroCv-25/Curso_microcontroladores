#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"

#include "esp_system.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "lwip/netdb.h"
#include "esp_smartconfig.h"

#include <wifi_provisioning/scheme_softap.h>

#include "wifi_app.h"

static const char TAG[] = "WIFI TASK";

// variável usada para contar o número de tentativas
static int g_retry_number;

// Fila utilizada para gerenciar as mensagens
static QueueHandle_t wifi_app_queue_handle;

/**
 * Função que é usada para gerenciar eventos
 * @param arg Informção passada pra Função quando ocorre o evento
 * @param event_base O tipo evento (Registramos qual tipo de evento viria para essa função nas configurações)
 * @param event_id O ID do evento se foi desconectado, conectado e outros tudo tem id de idetificação
 * @param event_data Dados do evento (pode ser a nossa senha ou o IP)
 */
static void wifi_app_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT)
    {
        switch (event_id)
        {
        case WIFI_EVENT_AP_START:
            ESP_LOGI(TAG, "WIFI_EVENT_AP_START");
            break;

        case WIFI_EVENT_AP_STOP:
            ESP_LOGI(TAG, "WIFI_EVENT_AP_STOP");
            break;

        case WIFI_EVENT_AP_STACONNECTED:
            ESP_LOGI(TAG, "WIFI_EVENT_AP_STACONNECTED");
            break;

        case WIFI_EVENT_AP_STADISCONNECTED:
            ESP_LOGI(TAG, "WIFI_EVENT_AP_STADISCONNECTED");
            break;

        case WIFI_EVENT_STA_START:
            ESP_LOGI(TAG, "WIFI_EVENT_STA_START");
            esp_wifi_connect();
            break;

        case WIFI_EVENT_STA_CONNECTED:
            ESP_LOGI(TAG, "WIFI_EVENT_STA_CONNECTED");
            break;

        case WIFI_EVENT_STA_DISCONNECTED:
            ESP_LOGI(TAG, "WIFI_EVENT_STA_DISCONNECTED");

            wifi_event_sta_disconnected_t *wifi_event_sta_disconnected = (wifi_event_sta_disconnected_t *)malloc(sizeof(wifi_event_sta_disconnected_t));
            *wifi_event_sta_disconnected = *((wifi_event_sta_disconnected_t *)event_data);
            printf("WIFI_EVENT_STA_DISCONNECTED, reason code %d\n", wifi_event_sta_disconnected->reason);

            if (g_retry_number < MAX_CONNECTION_RETRIES)
            {
                esp_wifi_connect();
                g_retry_number++;
            }
            else
            {
                wifi_app_send_message(WIFI_APP_MSG_STA_DISCONNECTED);
            }

            break;
        }
    }
    else if (event_base == IP_EVENT)
    {
        switch (event_id)
        {
        case IP_EVENT_STA_GOT_IP:
            ESP_LOGI(TAG, "IP_EVENT_STA_GOT_IP");

            wifi_app_send_message(WIFI_APP_MSG_STA_CONNECTED_GOT_IP);

            break;
        }
    }
    else if (event_base == SC_EVENT)
    {
        switch (event_id)
        {
        case SC_EVENT_SCAN_DONE:
            ESP_LOGI(TAG, "Scan done");
            break;

        case SC_EVENT_FOUND_CHANNEL:
            ESP_LOGI(TAG, "Found channel");
            break;

        case SC_EVENT_GOT_SSID_PSWD:
            ESP_LOGI(TAG, "Got SSID and password");

            smartconfig_event_got_ssid_pswd_t *evt = (smartconfig_event_got_ssid_pswd_t *)event_data;
            wifi_config_t wifi_config;
            uint8_t ssid[33] = {0};
            uint8_t password[65] = {0};
            uint8_t rvd_data[33] = {0};

            bzero(&wifi_config, sizeof(wifi_config_t));
            memcpy(wifi_config.sta.ssid, evt->ssid, sizeof(wifi_config.sta.ssid));
            memcpy(wifi_config.sta.password, evt->password, sizeof(wifi_config.sta.password));
            wifi_config.sta.bssid_set = evt->bssid_set;
            if (wifi_config.sta.bssid_set == true)
            {
                memcpy(wifi_config.sta.bssid, evt->bssid, sizeof(wifi_config.sta.bssid));
            }

            memcpy(ssid, evt->ssid, sizeof(evt->ssid));
            memcpy(password, evt->password, sizeof(evt->password));
            ESP_LOGI(TAG, "SSID:%s", ssid);
            ESP_LOGI(TAG, "PASSWORD:%s", password);
            if (evt->type == SC_TYPE_ESPTOUCH_V2)
            {
                ESP_ERROR_CHECK(esp_smartconfig_get_rvd_data(rvd_data, sizeof(rvd_data)));
                ESP_LOGI(TAG, "RVD_DATA:");
                for (int i = 0; i < 33; i++)
                {
                    printf("%02x ", rvd_data[i]);
                }
                printf("\n");
            }

            ESP_ERROR_CHECK(esp_wifi_disconnect());
            ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
            esp_wifi_connect();
            break;

        case SC_EVENT_SEND_ACK_DONE:

            break;

        default:
            break;
        }
    }
}

/**
 * @brief Função que configura a manipulação dos eventos pela função wifi_app_event_handler
 *
 */
static void wifi_app_event_handler_init(void)
{
    // Event loop for the WiFi driver
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Event handler for the connection
    esp_event_handler_instance_t instance_smart_config_event;
    esp_event_handler_instance_t instance_wifi_event;
    esp_event_handler_instance_t instance_ip_event;

    /* Register our event handler for Wi-Fi, IP and Provisioning related events */
    ESP_ERROR_CHECK(esp_event_handler_instance_register(SC_EVENT, ESP_EVENT_ANY_ID, &wifi_app_event_handler, NULL, &instance_smart_config_event));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_app_event_handler, NULL, &instance_wifi_event));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, ESP_EVENT_ANY_ID, &wifi_app_event_handler, NULL, &instance_ip_event));
}

/**
 * @brief Esse Função está disponivel para todos para podermos controlar a conexão em outro ponto do programa
 *
 * @param msgID A mensagem que vamos utilizar aqui por enquanto é WIFI_APP_MSG_USER_REQUESTED_CONNECTION
 * @return BaseType_t
 */
BaseType_t wifi_app_send_message(wifi_app_message_e msgID)
{
    wifi_app_queue_message_t msg;
    msg.msgID = msgID;
    return xQueueSend(wifi_app_queue_handle, &msg, portMAX_DELAY);
}

/**
 * @brief Nossa task wifi ela que vai receber as mensagens e decidir o que fazer
 * Para isso temos que fazer algumas configurações
 *
 * @param pvParameters
 */
static void wifi_app_task(void *pvParameters)
{
    // Nossa msg que vamos receber na fila
    wifi_app_queue_message_t msg;

    // configura a manipulação dos eventos pela função wifi_app_event_handler
    wifi_app_event_handler_init();

    // Inicializa o TCP IP
    ESP_ERROR_CHECK(esp_netif_init());

    /* Fazemos a configuração de Hardware em Station mode*/
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);

    /*habilitamos o hardware do  WIFI*/
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    /*Inicializamos WIFI em Station Mode*/
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    while (true)
    {
        // Esperamos receber as mensagens aqui da fila
        if (xQueueReceive(wifi_app_queue_handle, &msg, portMAX_DELAY))
        {
            switch (msg.msgID)
            {
                // Iniciamos o Smart Config
            case WIFI_APP_MSG_SMART_CONFIG_START:
                ESP_LOGI(TAG, "WIFI_APP_MSG_SMART_CONFIG_START");

                ESP_ERROR_CHECK(esp_smartconfig_set_type(SC_TYPE_ESPTOUCH));
                smartconfig_start_config_t cfg_smart_config = SMARTCONFIG_START_CONFIG_DEFAULT();
                ESP_ERROR_CHECK(esp_smartconfig_start(&cfg_smart_config));

                break;
                // Iniciamos o Recebemos as informações
            case WIFI_APP_MSG_SMART_CONFIG_CRED_RECV:
                ESP_LOGI(TAG, "WIFI_APP_MSG_SMART_CONFIG_CRED_RECV");
                break;

                // Iniciamos o SMART config finalizando
            case WIFI_APP_MSG_SMART_CONFIG_DONE:
                ESP_LOGI(TAG, "WIFI_APP_MSG_SMART_CONFIG_DONE");
                esp_smartconfig_stop();
                break;

                // Iniciamos o conectado
            case WIFI_APP_MSG_STA_CONNECTED_GOT_IP:
                ESP_LOGI(TAG, "WIFI_APP_MSG_STA_CONNECTED_GOT_IP");
                break;

                // Dessconectado o MCU vai reiniciar
            case WIFI_APP_MSG_STA_DISCONNECTED:
                ESP_LOGI(TAG, "WIFI_APP_MSG_STA_DISCONNECTED: Conexão perdida, verifique o seu Wi-Fi");
                vTaskDelay(30000 / portTICK_PERIOD_MS);
                esp_restart();
                break;

            // Usuário pediu para fazer uma nova conexão
            case WIFI_APP_MSG_USER_REQUESTED_CONNECTION:
                ESP_LOGI(TAG, "Usuário Pedindo para fazer uma nova conexão.");
                esp_wifi_disconnect();
                wifi_app_send_message(WIFI_APP_MSG_SMART_CONFIG_START);
                break;
            }
        }
    }
}

/**
 * @brief Configura e inicia a tarefa que cuida do WI-Fi
 *
 */
void wifi_app_start(void)
{
    ESP_LOGI(TAG, "Iniciando a aplicação");

    // Desabilitando mensagens da biblioteca do wifi
    esp_log_level_set("wifi", ESP_LOG_NONE);

    // criando a fila
    wifi_app_queue_handle = xQueueCreate(3, sizeof(wifi_app_queue_message_t));

    // Iniciando a task no core 1 com 4096 de memoria e prioridade 3
    xTaskCreatePinnedToCore(&wifi_app_task, "wifi_app_task", 4096, NULL, 3, NULL, 1);
}