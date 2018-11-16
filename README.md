# BH OnOfre
 
* [Intro](#id1)
* [Hardware](#id2)
* [Firmware](https://github.com/brunohorta82/easy_iot)
* [Video Tutoriais - YouTube](https://www.youtube.com/watch?v=OZenBfHWtak&list=PLxDLawCWayzDqAgOpIDJ-DHFAXYd_S-pr)
* [Donativos](#id6)
* [Site Oficial](http://bhonofre.pt/)



## Intro <a name="id1"></a>
O projeto BH OnOfre, é uma solução IoT baseada em ESP8266 com o objetivo de tornar a automação domiciliar muito mais simples, de forma aberta e sem restrições. Todo o código fonte é aberto bem como Hardware.

**BH OnOfre Easy Light** é um dispositivo baseado em ESP8266 com 2 relés de estado sólido cada um de 2A, este permite controlar até 2 circuitos de iluminação independentes. O facto de possuir relés de estado sólido permite ligar e desligar luzes de forma silênciosa.
O OnOfre pode ser instalado atrás dos interruptores normais, possui conectores para ligar o interruptor normal, ou sensor touch. Para alem dos conectores dedicados, permite ainda ligar os mais diversos sensores/atuadores aos GPIO's disponiveis. 
O controlo do mesmo pode ser efectuado via Web, MQTT ou manualmente pelo interruptor, tem integração automatica com o Home Assistant.
Mais info em [Easy Light REPO](https://github.com/brunohorta82/BH_OnOfre/tree/master/OnOfre%20EasyLight)

## Bibliotecas necessárias <a name="id3"></a>
  

Nome | Link | Versão 
:---: | :---: | ---:
JustWifi | [GIT](https://github.com/xoseperez/justwifi) | `last`
Timming | [GIT](https://github.com/scargill/Timing) | `last`
Async Mqtt Client | [GIT](https://github.com/marvinroger/async-mqtt-client) | `last`
ESP Async TCP | [GIT](https://github.com/me-no-dev/ESPAsyncTCP)| `last`
ESP Async Webserver | [GIT](https://github.com/me-no-dev/ESPAsyncWebServer) | `last`
Bounce2 | [GIT](https://github.com/thomasfredericks/Bounce2) | `last`
DHT Async | [GIT](https://github.com/brunohorta82/DHT_nonblocking) | `last`
Arduino Json | [ARDUINO IDE](https://arduinojson.org) | `5.*`



## Painel de Controlo <a name="id3"></a>

![dash](docs/onofre_v1_dash.png)
![devices](docs/onofre_v1_devices.png)

## Hardware <a name="id2"></a>
* BH OnOfre Dev Board 3.4
![dev_board_3.4](docs/onofre_dev_board_v3.4.png)


## Software necessário <a name="id3"></a>

- Arduino Ide (caso seja para alterar o codigo fonte)
- Upload do Firmware para o ESP8266 (dentro de cada projeto o firmware está dentro da pasta binarios)
- Aceder ao AP `SSID:BH_ONFFRE_NODE_ID` e configurar a Rede Wi-Fi
- Navegar até  `http://<bh-onofre name>.local` ou `http://<bh-onofre IP address>` ou `http://192.168.4.1` ligado directamente ao módulo
- Configurar o `Wi-Fi broker`, `MQTT broker` entre outras coisas como integração automática com `Home Assistant`
- E tá feito, agora é só curtir :) 

#Caixas e Adaptadores
 - ![Onofre Livolo Powered by Michaël Memeteau](https://cad.onshape.com/documents/edaf7fef18d999cab7aa6aff/w/efbde367d3f13fc563e8fda8/e/a555c81f28f9aefdf7ebc964)
 - ![Onofre Protecção Eléctrica Powered by Bruno Horta](https://cad.onshape.com/documents/2658804f4fec38f38b567684/w/01661975a7b4ba3d280e6216/e/88d1bd909d651dac55006571)

## Donativos <a name="id6"></a>

Se gostaste do projeto podes fazer o teu donativo :).

[![Donativo](https://img.shields.io/badge/Donate-PayPal-green.svg)](https://www.paypal.me/bhonofre)
