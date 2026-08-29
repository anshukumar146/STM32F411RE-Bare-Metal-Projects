#include <stdint.h>

#define RCC     0x40023800
#define GPIOA   0x40020000
#define GPIOB   0x40020400
#define GPIOC   0x40020800
#define ADC1    0x40012000
#define USART1  0x40011000

#define RCC_AHB1ENR   ((volatile uint32_t *)(RCC   + 0x30))
#define RCC_APB2ENR   ((volatile uint32_t *)(RCC   + 0x44))
#define RCC_APB1ENR   ((volatile uint32_t *)(RCC   + 0x40)) //For USART1

#define GPIOA_MODER ((volatile uint32_t *)(GPIOA + 0x00))
#define GPIOA_ODR   ((volatile uint32_t *)(GPIOA + 0x14))

#define GPIOB_MODER ((volatile uint32_t *)(GPIOB + 0x00))
#define GPIOB_AFRL  ((volatile uint32_t *)(GPIOB + 0x20)) //FOR USART1

#define GPIOC_MODER ((volatile uint32_t *)(GPIOC + 0x00))
#define GPIOC_ODR   ((volatile uint32_t *)(GPIOC + 0x14))

#define USART1_CR1     ((volatile uint32_t *)(USART1 + 0x0C))
#define USART1_SR      ((volatile uint32_t *)(USART1 + 0x00))
#define USART1_DR      ((volatile uint32_t *)(USART1 + 0x04))
#define USART1_BRR     ((volatile uint32_t *)(USART1 + 0x08))

#define ADC_CR1     ((volatile uint32_t *)(ADC1 + 0x04))
#define ADC_CR2     ((volatile uint32_t *)(ADC1 + 0x08))
#define ADC_SMPR2   ((volatile uint32_t *)(ADC1 + 0x10))
#define ADC_SQR1    ((volatile uint32_t *)(ADC1 + 0x2C))
#define ADC_SQR3    ((volatile uint32_t *)(ADC1 + 0x34))
#define ADC_SR      ((volatile uint32_t *)(ADC1 + 0x00))
#define ADC_DR      ((volatile uint32_t *)(ADC1 + 0x4C))
#define ADC_CCR     ((volatile uint32_t *)((ADC1 + 0x300) + 0x04 ))



void GPIO_Init(void);
void USART1_INIT(void);
void ADC_Init(void);
uint16_t ADC_Read(void);
void USART1_SendADC(uint16_t ADC_value);

void USART1_SendChar(char data);
char USART1_ReceiveChar(void);
void ASCII_conversion(uint16_t ADC_value);

void delay(uint32_t n);
void LCD_SendNibble(uint8_t nibble);
void LCD_Command(uint8_t command);
void LCD_Data(uint8_t data);
void LCD_Init(void);
void LCD_DisplayADC(uint16_t ADC_value);



void delay(uint32_t n)
{
    for (volatile uint32_t i = 0; i < n; i++)
    {
    }
}

void LCD_SendNibble(uint8_t nibble)
{
    uint8_t D4;
    uint8_t D5;
    uint8_t D6;
    uint8_t D7;

    D7   = ((nibble >> 3) & 1);
    D6   = ((nibble >> 2) & 1);
    D5   = ((nibble >> 1) & 1);
    D4   = ((nibble >> 0) & 1);

    *GPIOA_ODR &= ~((1 << 6)|(1 << 5));
    *GPIOA_ODR |= ((D6 << 6)|(D7 << 5));

    *GPIOC_ODR &= ~((1 << 0)|(1 << 1));
    *GPIOC_ODR |= ((D4 << 0)|(D5 << 1));


    *GPIOA_ODR |=  (0b1 << 4);
    delay(5000);

    *GPIOA_ODR &= ~(0b1 << 4);
    delay(5000);
}


void LCD_Command(uint8_t command)
{
    *GPIOA_ODR &= ~(0b1 << 1);

    uint8_t high_nibble = ((command >> 4) & 0x0F);
    LCD_SendNibble(high_nibble);

    uint8_t low_nibble = ((command >> 0) & 0x0F);
    LCD_SendNibble(low_nibble);

    delay(1000);
}


void LCD_Data(uint8_t data)
{
    *GPIOA_ODR |= (0b1 << 1);

    uint8_t high_nibble = ((data >> 4) & 0x0F);
    LCD_SendNibble(high_nibble);

    uint8_t low_nibble = ((data >> 0) & 0x0F);
    LCD_SendNibble(low_nibble);

    delay(2000);
}

void LCD_Init(void)
{
    delay(100000);

    *GPIOA_ODR &= ~(1 << 1);

    LCD_SendNibble(0x03);
    delay(20000);

    LCD_SendNibble(0x03);
    delay(10000);

    LCD_SendNibble(0x03);
    delay(5000);

    LCD_SendNibble(0x02);
    delay(5000);

    LCD_Command(0x28);
    LCD_Command(0x0C);
    LCD_Command(0x06);
    LCD_Command(0x01);

    delay(30000);
}

void LCD_DisplayADC(uint16_t ADC_value)
{
    uint16_t Thousands;
    uint16_t Hundreds;
    uint16_t Tens;
    uint16_t Ones;

    Thousands = ADC_value / 1000;
    Hundreds  = (ADC_value / 100) % 10;
    Tens      = (ADC_value / 10) % 10;
    Ones      = ADC_value % 10;

    LCD_Data('A');
    LCD_Data('D');
    LCD_Data('C');
    LCD_Data('=');



    LCD_Data(Thousands + '0');
    LCD_Data(Hundreds + '0');
    LCD_Data(Tens + '0');
    LCD_Data(Ones + '0');

    LCD_Command(0xC0);

    LCD_Data('L');
    LCD_Data('E');
    LCD_Data('D');
    LCD_Data('=');


    if(ADC_value < 2000)
    {
    LCD_Data('O');
    LCD_Data('N');
    LCD_Data(' ');
    }
    else
    {
    LCD_Data('O');
    LCD_Data('F');
    LCD_Data('F');
    }

}


void GPIO_Init(void)
{
    *RCC_AHB1ENR |= ((1 << 1)|(1 << 0)|(1 << 2));

    *GPIOC_MODER &= ~((0b11 << 0) | (0b11 << 2));
    *GPIOC_MODER |=  ((0b01 << 0) | (0b01 << 2));

    *GPIOA_MODER &= ~((0b11 << 0)|
                    (0b11 << 2)|
                    (0b11 << 8)|
                    (0b11 << 10)|
                    (0b11 << 12));


    *GPIOA_MODER |=  ((0b11 << 0)|
                    (0b01 << 2)|
                    (0b01 << 8)|
                    (0b01 << 10)|
                    (0b01 << 12));

}


void USART1_SendADC(uint16_t ADC_value)
{
    uint8_t highByte = (ADC_value >> 8) & 0xFF;
    uint8_t lowByte = ADC_value & 0xFF;
    uint8_t checksum = highByte ^ lowByte;

    USART1_SendChar(0xAA);
    USART1_SendChar(highByte);
    USART1_SendChar(lowByte);
    USART1_SendChar(checksum);
}


void USART1_INIT(void)
{
    *RCC_APB2ENR |= (1 << 4);

    *GPIOB_MODER &= ~((0b11 << 12)|(0b11 << 14));
    *GPIOB_MODER |=  ((0b10 << 12)|(0b10 << 14));

    *GPIOB_AFRL &= ~((0b1111 << 24)|(0b1111 << 28));
    *GPIOB_AFRL |=  ((0b0111 << 24)|(0b0111 << 28));

    *USART1_CR1 &= ~(0b1 << 15);


    *USART1_BRR = ((104 << 4) | (3 << 0));     //BRR =9600


    *USART1_CR1  &= ~((0b1 << 3)| (0b1 << 2)| (0b1 << 13));
    *USART1_CR1  |=  ((0b1 << 3)|(0b1 << 2));
    *USART1_CR1  |=  (0b1 << 13);

}


void ADC_Init(void)
{
    *RCC_APB2ENR |= (1 << 8);
    *ADC_CR1     &= ~(0b11 << 24);
    *ADC_SMPR2   &= ~(0b111 << 0);
    *ADC_SMPR2   |=  (0b100 << 0);
    *ADC_SQR1    &= ~(0b1111 << 20);
    *ADC_SQR1    |=  (0b0000 << 20);
    *ADC_SQR3    &= ~(0b11111 << 0);
    *ADC_SQR3    |=  (0b00000 << 0);

    *ADC_CR2 &= ~((0b1 << 0)|
                (0b1 << 1)  |
                (0b1 << 30) |
                (0b1 << 11) |
                (0b1 << 10) |
                (0b11 << 28)|
                (0b1 << 8)  |
                (0b1 << 9));

    *ADC_CR2 |= ((0b1 << 0)|
                (0b0 << 1)  |
                (0b0 << 11) |
                (0b0 << 10) |
                (0b00 << 28));


    *ADC_CCR &= ~(0b11 << 16);
    *ADC_CCR |=  (0b11 << 16);

}

uint16_t ADC_Read(void)
{
    *ADC_CR2 |= (0b1 << 30);

    while ((*ADC_SR >> 1) == 0)
    {
    }

    return (*ADC_DR);
}


void USART1_SendChar(char data)
{
  while ((( *USART1_SR >> 7 ) & 1) == 0)
   {

   }
  *USART1_DR = data;

}

char USART1_ReceiveChar(void)
{
  while ((( *USART1_SR >> 5 ) & 1) == 0)
   {

   }
  char receivedata = *USART1_DR;

   return receivedata;
}



void ASCII_conversion(uint16_t ADC_value)
{
    uint16_t Thousands;
    uint16_t Hundreds;
    uint16_t Tens;
    uint16_t Ones;

    Thousands = ADC_value / 1000;
    Hundreds  = (ADC_value / 100) % 10;
    Tens      = (ADC_value / 10) % 10;
    Ones      = ADC_value % 10;

    USART1_SendChar(Thousands + '0');
    USART1_SendChar(Hundreds + '0');
    USART1_SendChar(Tens + '0');
    USART1_SendChar(Ones + '0');
    USART1_SendChar('\r');
    USART1_SendChar('\n');
}


int main()
{
    GPIO_Init();
    ADC_Init();
    USART1_INIT();

    LCD_Init();



    while(1)
    {
        uint16_t ADC_value = ADC_Read();

        LCD_Command(0x80);
        LCD_DisplayADC(ADC_value);

        USART1_SendADC(ADC_value);

        delay(300000);
    }
}
