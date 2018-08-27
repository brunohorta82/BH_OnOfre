<b>Pedimos desculpa estamos em manutenção do repositorio</b>

# BH OnOfre


**Conteúdo **   
1. [Intro](#id1)
2. [Hardware](#id2)
3. [Software](#id3)
4. [Donativos](#id6)



## Intro <a name="id1"></a>
O projeto BH OnOfre, é uma solução IoT baseada em ESP8266 com o objetivo de tornar a automação domiciliar muito mais simples, de forma aberta e sem restrições. Todo o código fonte é aberto bem como Hardware.

**BH OnOfre Easy Light** é um dispositivo baseado em ESP8266 com 2 relés de estado sólido, que permite controlar até 2 circuitos de iluminação independentes. O facto de possuir relés de estado sólido permite ligar e desligar luzes de forma silênciosa.
O OnOfre pode ser instalado atrás dos interruptores normais, possui conectores para ligar o interruptor normal, ou sensores touch. Para alem dos conectores dedicados, permite ainda ligar os mais diversos sensores aos GPIO's disponiveis. 
O controlo do mesmo pode ser efectuado via Web, MQTT ou manualmente pelo interruptor, tem integração automatica com o Home Assistant.

**Painel de Controlo:**



**Relés :**
  - Controlados via MQTT
  - WEB 
  - GPIO's

**ESP8266**:
  - Facilmente configurado na Interface Web

  
## Hardware <a name="id2"></a>





Qtd | Item | Imagem
:---: | :---: | ---:
1 | ESP-12E | `...`
1 | PCB BH-OnOfre Easy Light | `...`
1 | Fonte HLK-PM01 | `...`
1 | Regulador de Tensão AMS1117 3.3v | `...`
1 | Condensador Cerâmico 100nF SMD | `...`
5 | Resistência 10K 0603 | `...`
2 | Botão 3*4*2 MM SMD | `...`
1 | Fusível T2A 2A 250 | `...`
1 | Terminal de Parafuso 4P 5.0mm  | `...`
1 | Conector de 9 Pinos Fêmea 2.54mm | `...`
1 | Conector de 8 Pinos Fêmea 2.54mm | `...`



## Software <a name="id3"></a>

- Arduino Ide
- Editar o ficheiro config.h com o setup inicial
- Upload do Firmware para o ESP8266
- Aceder ao AP `SSID:BH_ONFFRE_NODE_ID` e configurar a Rede Wi-Fi
- Navegar até  `http://<bh-onofre name>.local` ou `http://<bh-pzem IP address>`
- Configurar o `MQTT broker`
- E tá feito, agora é só curtir :) 


## Donativos <a name="id6"></a>

Se gostas-te do projeto podes fazer o teu donativo :).

[![Donativo](https://img.shields.io/badge/Donate-PayPal-green.svg)](https://www.paypal.me/bhonofre)
