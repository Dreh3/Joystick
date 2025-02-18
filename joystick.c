//Código desenvolcido por Andressa Sousa Fonseca

/*
O presente código permite que a intensidade dos leds sejam controladas pela posição dos eixos do joystick,
de acordo com os valores adc:
Em 2048 os leds desligam, level 0. Entre 0 e 2048 e entre 2048 e 4096 os leds variam de intensidade, 
aumentando em direção aos extremos. Sendo em 0 e 2048, a intensidade máxima. 
O eixo X controla o led Azul e o eixo Y controla o led Vermelho.
Os botões tem as seguintes funcionalidades:
Botão A: habilita e desabilta leds Azul e Vermelho. (Leds PWM)
Botão do Joystick: Alterna o estado do led Verde e modifica as bordas do display.
Por último, no display, um quadrado movimenta-se de acordo com as coordenadas do joystick.
*/

#include <stdio.h> //Biblioteca padrão de C que permite entrada e saída de dados
#include "pico/stdlib.h" //Biblioteca fundamental do SDK
#include "hardware/pwm.h" //Biblioteca para manipular o pwm
#include "hardware/adc.h" //Biblioteca para as configurações do adc
#include "math.h"
//Configurações para o displey
#include "hardware/i2c.h" //Biblioteca para comunicação i2c
#include "ssd1306.h"
#include "image.h"

#define ledG 11 //Led Verde no pino 11
#define ledB 12 //Led Azul no pino 12
#define ledR 13 //Led Vermelho no pino 13

#define Button_Joy 22 //Botão do joystick no pino 22
#define ButtonA 5 //Botão A no pino 5

//Entradas adc do joystick
#define VX_JOY 26 //Entrada das coordernadas x
#define VY_JOY 27 //Entrada das coordernadas y

#define I2C_PORT i2c1
#define I2C_SDA 14 //pinos para comunicação I2C
#define I2C_SCL 15 //pinos para comunicação I2C
#define endereco 0x3C
ssd1306_t ssd; // Inicializa a estrutura do display

//Variáveis para configurar o pwm
#define DIVIDER_PWM 100 //Estabelecendo divisor de clock
#define WRAP_PERIOD 24999 //Com o divisor do clock em 100, o wrap tem valor 24999 
uint16_t levelX = 0; //Nível inicial baixo
uint16_t levelY=0; //Inica com os display apagados

//Declaração de variáveis necessárias
static volatile uint32_t tempo_anterior = 0; //Para o debouncing
static volatile bool estado_leds = true; //Para habilitar ou desabilitar mudanças nos leds PWM
static volatile char tipo_borda = 'A'; //Para selecionar as bordas do display

//Função para configurar os leds
void led_config(uint led_pin){
  gpio_init(led_pin);
  gpio_set_dir(led_pin, GPIO_OUT);
  gpio_put(led_pin, 0);
};

//Função para configurar os botões
void botao_config(uint pino){
  gpio_init(pino);
  gpio_set_dir(pino, GPIO_IN);
  gpio_pull_up(pino);
};

//Função para inicializar configurações do pwm
void config_pwm(int pino){  //Função pode ser usada para o led e o servo 
    uint slice;
    gpio_set_function(pino, GPIO_FUNC_PWM); //Configura pino do led como pwm
    slice = pwm_gpio_to_slice_num(pino); //Adiquire o slice do pino
    pwm_set_clkdiv(slice, DIVIDER_PWM); 
    pwm_set_wrap(slice, WRAP_PERIOD);
    pwm_set_gpio_level(pino, 0); //Determina com o level desejado
    pwm_set_enabled(slice, true);
};

//Função para limpar display
void limpar_display(){
  bool cor = true; //Controla cor do display para o fundo contrastar com letra
  cor = !cor;
  ssd1306_fill(&ssd, !cor); // Limpa o display
}

// Prototipo da função de interrupção
static void interrupcao_Botao(uint gpio, uint32_t events);

int main()
{
    //Declarando veriáveis necessárias
    uint16_t valor_atualX; //Valores do eixo x
    uint16_t valor_atualY; //Valores do eixo y
    uint linhaX; //Armazenará valores de posição no eixo x do display
    uint linhaY; //Armazenará valores de posição no eixo y do display

    stdio_init_all(); //Inicia comunicação padrão
    adc_init(); //Inicializa o adc
    //Inicializando entradas analógicas
    adc_gpio_init(VX_JOY); 
    adc_gpio_init(VY_JOY);

    // Inicializando comunicação I2C
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C); // Configura pino SDA para I2C
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C); // Configura pino SCL para I2C

    //Configurando leds como saída PWM
    led_config(ledB);
    led_config(ledR);
    config_pwm(ledB);
    config_pwm(ledR);

    //Configurando o led verde
    led_config(ledG); 

    //Configurando botões
    botao_config(Button_Joy);
    botao_config(ButtonA);

    // Inicializa o display
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); 
    ssd1306_config(&ssd); // Configura o display
    ssd1306_send_data(&ssd); // Envia os dados para o display
    
    bool cor = true;    //Controla cor do display para o fundo contrastar com letra
    cor = !cor;
    ssd1306_draw_image(&ssd, 'Q', 120, 56); //Inicia com o quadrado no centro sem borda
    ssd1306_send_data(&ssd); // Atualiza o display

    //Configurando interrupções para os botões
    gpio_set_irq_enabled_with_callback(ButtonA, GPIO_IRQ_EDGE_FALL,true, interrupcao_Botao);
    gpio_set_irq_enabled_with_callback(Button_Joy, GPIO_IRQ_EDGE_FALL,true, interrupcao_Botao);

    while (true) {
      //Selecionando o canal e lendo o valor correpondente
      adc_select_input(1);
      valor_atualX = abs(adc_read() - 4095);
      adc_select_input(0);
      valor_atualY = adc_read();

      //Converte os valores adc para posições correpondentes no display
      linhaX = (120*4096 - (120*valor_atualX))/4096;
      linhaY = (56*4096 - (56*valor_atualY))/4096;

      //Atualizando display
      ssd1306_fill(&ssd, cor); // Limpa o display
      //Desenha bordas
      if(tipo_borda=='A'){ //Duas bordas horizontais
        ssd1306_line(&ssd, 4, 127, 124, 127, !cor); //horizontal
        ssd1306_line(&ssd, 4, 1, 124, 1, !cor); //horizontal
      }else if(tipo_borda=='B'){ //Duas bordas verticais
        ssd1306_line(&ssd, 126, 0, 127, 126, !cor); //vertical
        ssd1306_line(&ssd, 1, 127, 1, 0, !cor);
      }else if(tipo_borda=='C'){ //Uma horizontal e uma vertical
        ssd1306_line(&ssd, 4, 127, 124, 127, !cor); //horizontal
        ssd1306_line(&ssd, 126, 0, 127, 126, !cor); //vertical
      }else if(tipo_borda=='D'){ //Uma horizontal e uma vertical
        ssd1306_line(&ssd, 4, 1, 124, 1, !cor); //horizontal
        ssd1306_line(&ssd, 1, 127, 1, 0, !cor);
      }else{ //Um retângulo
        ssd1306_line(&ssd, 4, 127, 124, 127, !cor); //horizontal
        ssd1306_line(&ssd, 4, 1, 124, 1, !cor); //horizontal
        ssd1306_line(&ssd, 126, 0, 127, 126, !cor); //vertical
        ssd1306_line(&ssd, 1, 127, 1, 0, !cor);
      };
      ssd1306_draw_image(&ssd, 'Q', linhaX, linhaY);
      ssd1306_send_data(&ssd); // Atualiza o display

      //Modifica intensidade dos leds se eles estiverem habilitados
      if(estado_leds){
        if (valor_atualX ==0 || valor_atualX == 2048){
            levelX = 24999;
        }else if(valor_atualX < 2048) {
          levelX = ((2048 - valor_atualX) * WRAP_PERIOD) / 2048; 
        } else if (valor_atualX > 2048) {
            levelX = ((valor_atualX - 2048) * WRAP_PERIOD) / 2048; 
        }else {
            levelX = 0; // Apaga quando estiver em 2048
        };

        if (valor_atualY ==0 || valor_atualY == 2048){
            levelY = 24999;
        }else if(valor_atualY < 2048) {
          levelY = ((2048 - valor_atualY) * WRAP_PERIOD) / 2048; 
        } else if (valor_atualY > 2048) {
            levelY = ((valor_atualY - 2048) * WRAP_PERIOD) / 2048; 
        }else {
            levelY = 0; // Apaga quando estiver em 2048
        };
        printf("Intensidade X: %d; VX: %d\n", levelX, valor_atualX);
        pwm_set_gpio_level(ledB, levelX); //Atualiza o level
        printf("Intensidade Y: %d; VY: %d\n", levelY, valor_atualY);
        pwm_set_gpio_level(ledR, levelY); //Atualiza o level        
      };

      sleep_ms(10); //Pausa para atualizar e suavizar as mudanças
    };
};

void interrupcao_Botao(uint gpio, uint32_t events){
   // Obtém o tempo atual em microssegundos
  uint32_t tempo_atual = to_us_since_boot(get_absolute_time());
  uint level =0;      //Para desligar os leds
   if(tempo_atual - tempo_anterior > 300000){ //300ms de deboucing
      tempo_anterior=tempo_atual; //Atualiza valor
      if(gpio==ButtonA){ //Se for o botão A, deliga os leds PWM
        estado_leds = !estado_leds;
        pwm_set_gpio_level(ledB, level); //Atribui level 0 aos leds
        pwm_set_gpio_level(ledR, level);

      }else if (gpio==Button_Joy){ //Se for o botão do joystick
          gpio_put(ledG,!gpio_get(ledG)); //Alterna o estado do led verde
          //Alterar borda do display
          switch (tipo_borda) //Modifica a borda do display
          {
          case 'A':
            tipo_borda = 'B';
            break;
          case 'B':
            tipo_borda = 'C';
            break;
          case 'C':
            tipo_borda = 'D';
            break;
          case 'D':
            tipo_borda = 'E';
            break;
          case 'E':
            tipo_borda = 'A';
            break;
          default:
            break;
          };
      };
   }; 

};