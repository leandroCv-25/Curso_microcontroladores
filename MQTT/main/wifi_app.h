#ifndef WIFI_APP_H_
#define WIFI_APP_H_

#include "freertos/FreeRTOS.h"

#define MAX_CONNECTION_RETRIES		5	


// Callback typedef
typedef void (*wifi_connected_event_callback_t)(void);

/**
 * @brief Mensagens que irão aparecer nos eventos para podermos resolver as situações
 * 
 */
typedef enum wifi_app_message
{
	WIFI_APP_MSG_SMART_CONFIG_START, //Começou o Smart Config
	WIFI_APP_MSG_SMART_CONFIG_CRED_RECV, //Recebeu a senha
	WIFI_APP_MSG_SMART_CONFIG_DONE, //Conectou ao Wifi e esperando o IP
	WIFI_APP_MSG_STA_CONNECTED_GOT_IP, //Conectou e conseguiu IP
	WIFI_APP_MSG_STA_DISCONNECTED, //WI-FI foi desconectado
	WIFI_APP_MSG_USER_REQUESTED_CONNECTION, // O usuário vai pedir para Ligar o Smart Config
} wifi_app_message_e;

/**
 * @brief Como a mensagem é armazenada em uma variavel, poderiamos transmitir mais informação
 * 
 */
typedef struct wifi_app_queue_message
{
	wifi_app_message_e msgID;
} wifi_app_queue_message_t;

/**
 * @brief Esse Função está disponivel para todos para podermos controlar a conexão em outro ponto do programa
 * 
 * @param msgID A mensagem que vamos utilizar aqui por enquanto é WIFI_APP_MSG_USER_REQUESTED_CONNECTION
 * @return BaseType_t 
 */
BaseType_t wifi_app_send_message(wifi_app_message_e msgID);

/**
 * @brief Configura e inicia a tarefa que cuida do WI-Fi
 * 
 */
void wifi_app_start(wifi_connected_event_callback_t cb);

#endif