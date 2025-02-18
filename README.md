# Joystick

__Tarefa - Aula Síncrona 10/02__<br>
Repositório criado com o intuito de realizar a Tarefa da aula Síncrona do dia 10 de fevereiro de 2025 sobre Conversores A/D.

__Responsável pelo desenvolvimento:__
Andressa Sousa Fonseca

## Descrição Da Tarefa 
__Aplicação de Comunicação serial I2C, Display 128x64,Leds RGB, PWM e Conversores A/D__  <br>

__Os Componentes necessários para a execução da atividade são:__
1) LED RGB, com os pinos conectados às GPIOs (11, 12 e 13).
2) Botão do Joystick conectado à GPIO 22.
3) Joystick conectado aos GPIOs 26 e 27.
4) Botão A conectado à GPIO 5.
5) Display SSD1306 conectado via I2C (GPIO 14 e GPIO15).

__As funcionalidade básicas especificadas para a atividade são:__
1)  O LED Azul terá seu brilho ajustado conforme o valor do eixo Y.
2) O LED Vermelho seguirá o mesmo princípio, mas de acordo com o eixo X.
3) Os LEDs serão controlados via PWM para permitir variação suave da intensidade luminosa.
4) Exibir no display SSD1306 um quadrado de 8x8 pixels.
5) O botão do joystick terá duas funcionalidades.
6) O botão A terá a seguinte funcionalidade: Ativar ou desativar os LED PWM a cada acionamento.


__Uma breve explicação do desenvolvimento e resultados obtidos podem ser vistos no vídeo endereçado no seguine link: [Aplicação de Conversores A/D](https://youtu.be/GN4ztbMzv6M?si=sReb9UBP103bOldQ).__

## Detalhamento Do Projeto

### 1. Controle de intensidade dos Leds

O projeto presente nesse repositório possui diversas funcionalidades. A primeira delas é controlar a intensidade dos Leds Vermelho e Azul de acordo com a posição dos eixos X e Y do Joystick, respectivamente. O controle de intensidade se dá por meio de PWM. Assim, os leds são configurados como saída PWM e os valores do eixos do Joystick são lidos por duas entradas, configuradas como entradas adc. <br>
Detalhes a relação entre os eixos e intensidade dos leds:
1. Quando os valor lido é igual a 2048, o led correspondente deve desligar.
2. Nas extremidades, os valores lidos são 0 e 4096 que darão aos leds a maior intensidade.
3. Entre 2048 e 0, a intensidade aumenta em direção ao 0. E entra 2048 e 4096 a intensidade aumenta em direção ao maior valor. <br>

Para permitir a conversão de valores do adc para os valores de intensidade desejados, a seguinte relação foi utilizada:

<div align="center">
  <img src="https://github.com/user-attachments/assets/c0de3dbc-b63b-4b27-854a-70cee4418c74" alt="adc para level" width="400"/>
</div>

### 2. Controle do quadrado no display
Ademais, o Joystick também tem a função de mover um quadrado 8x8 pelo display 128x64. Algumas configurações foram necessárias para que funcionasse corretamente:<br>

1. Relação entre o valores lidos nas portas analógicas e a posição no display
<div align="center">
  <img src="https://github.com/user-attachments/assets/a171e55a-6301-45c9-bb29-7f78c7baa526" alt="adc para posição" width="400"/>
</div>

2. Foi necessário inverter os canais no adc_select() <br>
O eixo X ficou sendo lido pelo canal 1 e o eixo Y pelo canal 0. <br>

```bash
adc_select_input(1);
valor_atualX = abs(adc_read() - 4095);
adc_select_input(0);
valor_atualY = adc_read();
```

3. Relação para posicionar o quadrado corretamente <br>
A seguinte relação para o eixo x foi usada para que a posição do quadrado no display fosse extamente igual a do joystick: 


### 3. Funcionalidades do botões

Os botões tem diferentes funções. O Botão A liga e desliga os Leds Vermelho e Azul. E o Botão do joystick alterna os estado do Led Verde e, também, perimite modificar as bordas exibidas no display. <br>
A utilização de dois botões permitiu implementar um tratamento de debouncing via software aliado a rotinas de interupção, detalhadas em aulas anteriores. Foi utilizada a seguinte função de interrupção:
```bash
static void interrupcao_Botao(uint gpio, uint32_t events);
```

Na main, a função acima é chamada pela função de interrupção com callback a seguir:

```bash
gpio_set_irq_enabled_with_callback(ButtonA, GPIO_IRQ_EDGE_FALL, true, &interrupcao_Botao);
gpio_set_irq_enabled_with_callback(Button_Joy, GPIO_IRQ_EDGE_FALL, true, &interrupcao_Botao);
```
É possível perceber que os dois botões chamam a mesma função, interrupcao_Botao(). O diferenciamento entre os botões é feito dentro da função a partir de um if e consideran-se um tempo de 300ms para aceitar que o botão foi pressionado.
