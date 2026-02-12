# _PWM_
##_-> Motor BDC_

A modulação por largura de pulso (PWM), também conhecida como modulação por duração de pulso (PDM) ou modulação por comprimento de pulso (PLM), é um método de controlar a potência média ou amplitude fornecida por um sinal elétrico.

Neste caso de estudo uma GPIO está sendo programada como saída de PWM e estamos a utilizando controlar a velocidade de um motor, utilizando o conceito de controle PID.

Nosso exemplo será suportado nos seguites devices:

| ESP32 | ESP32-C6 | ESP32-H2 | ESP32-P4 | ESP32-S3 |
| ----- | -------- | -------- | -------- | -------- | 

Os exemplos foram desenvolvidos utilizando o framework da espressif o esp-idf.