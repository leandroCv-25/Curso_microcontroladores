# _Exemplo TIMER_

Timers podem ser utilizados pelo programa para criar contadores e temporizadores que rodam no hardware e deixam o programa livre para executar outras tarefas em paralelo.

Neste caso de estudo é o uso do timer está sendo programada como saída e estamos a utilizando para acender um LED em um intervalo programado.

Vamos perceber o quanto o timer ajuda a ficar preciso, mesmo com o microcontrolador trabalhando em outros processos.

Nosso exemplo será suportado nos seguites devices:

| ESP32 | ESP32-C2 | ESP32-C3 | ESP32-C6 | ESP32-H2 | ESP32-S2 | ESP32-S3 |
| ----- | -------- | -------- | -------- | -------- | -------- | -------- |

Os exemplos foram desenvolvidos utilizando o framework da espressif o esp-idf.