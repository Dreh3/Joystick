# Joystick

__Tarefa - Aula Síncrona 10/02__<br>
Repositório criado com o intuito de realizar a Tarefa da aula Síncrona do dia 10 de fevereiro de 2025 sobre Conversores A/D.

__Responsável pelo desenvolvimento:__
Andressa Sousa Fonseca

## Descrição Da Tarefa 
__Aplicação de Comunicação serial I2C, Display 128x64,Leds RGB, PWM e Conversores A/D__  <br>

__Os Componentes necessários para a execução da atividade são:__
1) 

__As funcionalidade básicas especificadas para a atividade são:__
1) 


__Uma breve explicação do desenvolvimento e resultados obtidos podem ser vistos no vídeo endereçado no seguine link: [Aplicação de Conversores A/D]().__

## Detalhamento Do Projeto

### 1. Controle de intensidade dos Leds

O projeto presente nesse repositório possui diversas funcionalidades. A primeira delas é controlar a intensidade dos Leds Vermelho e Azul de acordo com a posição dos eixos Y e X do Joystick, respectivamente. O controle de intensidade se dá por meio de PWM. Assim, os leds são configurados como saída PWM e os valores do eixos do Joystick são lidos por duas entradas, configuradas como entradas adc. 
Detalhando relação eixos e intensidade dos leds:
1. Quando os valor lido é igual a 2048, o led correspondente deve desligar.
2. Nas extremidades, os valores lidos são 0 e 4096 que darão aos leds a maior intensidade.
3. Entre 2048 e 0, a intensidade aumenta em direção ao 0. E entra 2048 e 4096 a intensidade aumenta em direção ao maior valor.
Para permitir a conversão de valores do adc para os valores de intensidade desejados, a seguinte relação foi utilizada:

<div align="center">
  <img src="https://github.com/user-attachments/assets/c0de3dbc-b63b-4b27-854a-70cee4418c74" alt="adc para level" width="300"/>
</div>

### 2. Controle do quadrado no display
Ademais, o Joystick também tem a função de mover um quadrado 8x8 pelo display 128x64. Algumas configurações foram necessárias para que funcionasse corretamente:
1. Relação entre o valores lidos nas portas analógicas e a posição no display
<div align="center">
  <img src="https://github.com/user-attachments/assets/a171e55a-6301-45c9-bb29-7f78c7baa526" alt="adc para posição" width="300"/>
</div>
2. Foi necessário inverter os canais no adc_select()
O eixo X ficou sendo lido pelo canal 1 e o eixo Y pelo canal 0.
3. Relação para posicionar o quadrado corretamente
A seguinte relação foi usada para que a posição do quadrado no display fosse extamente igual a do joystick.
```bash
adc_select_input(1);
valor_atualX = abs(adc_read() - 4095);
adc_select_input(0);
valor_atualY = adc_read();
```

### 3. Funcionalidades do botões
A utilização de dois botões permitiu implementar um tratamento de debouncing via software aliado a rotinas de interupção, detalhadas em aulas anteriores. Foi utilizada a seguinte função de interrupção:

static void interrupcao_Botao(uint gpio, uint32_t events);
```
Na main, a função acima é chamada pela função de interrupção com callback a seguir:
```bash
gpio_set_irq_enabled_with_callback(ButtonA, GPIO_IRQ_EDGE_FALL, true, &interrupcao_Botao);
gpio_set_irq_enabled_with_callback(Button_Joy, GPIO_IRQ_EDGE_FALL, true, &interrupcao_Botao);
```
É possível perceber que os dois botões chamam a mesma função, interrupcao_Botao(). O diferenciamento entre os botões é feito dentro da função a partir de um if e consideran-se um tempo de 300ms para aceitar que o botão foi pressionado.
