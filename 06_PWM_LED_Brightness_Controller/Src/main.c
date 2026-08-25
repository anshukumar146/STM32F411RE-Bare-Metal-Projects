#include <stdint.h>


#define RCC   0x40023800
#define GPIOA 0x40020000
#define GPIOC 0x40020800
#define TIM2 0x40000000


#define AHB1ENR     ((volatile uint32_t *)(RCC + 0x30))
#define APB1ENR     ((volatile uint32_t *)(RCC +  0x40))

#define GPIOA_MODER ((volatile uint32_t *)(GPIOA + 0x00))
#define GPIOC_MODER ((volatile uint32_t *)(GPIOC + 0x00))

#define GPIOA_PUPDR ((volatile uint32_t *)(GPIOA + 0x0C))
#define GPIOC_PUPDR ((volatile uint32_t *)(GPIOC + 0x0C))

#define GPIOA_IDR   ((volatile uint32_t *)(GPIOA + 0x10))
#define GPIOC_IDR   ((volatile uint32_t *)(GPIOC + 0x10))

#define GPIOA_AFRL   ((volatile uint32_t *)(GPIOA + 0x20))

#define TIM2_CR1     ((volatile uint32_t *)(TIM2 + 0x00))
#define TIM2_EGR     ((volatile uint32_t *)(TIM2 + 0x14))

#define TIM2_CCMR1   ((volatile uint32_t *)(TIM2 + 0x18))
#define TIM2_CCER    ((volatile uint32_t *)(TIM2 + 0x20))
#define TIM2_CCR1    ((volatile uint32_t *)(TIM2 + 0x34))

#define TIM2_CNT     ((volatile uint32_t *)(TIM2 + 0x24))
#define TIM2_PSC     ((volatile uint32_t *)(TIM2 + 0x28))
#define TIM2_ARR     ((volatile uint32_t *)(TIM2 + 0x2C))



void GPIO_Init(void);
void TIM2_Init(void);
void Switch_Read(void);
void Delay(volatile uint32_t count);
uint8_t Switch_Debounce(volatile uint32_t *IDR, uint32_t pin);




void GPIO_Init(void){

*AHB1ENR |=  ((1 << 0 )|(1 << 2));

*GPIOA_MODER &= ~((0b11 << 2 ) |  (0b11 << 8 ) );
*GPIOC_MODER &= ~( (0b11 << 0 ) |  (0b11 << 2 ) |  (0b11 << 6 ));

*GPIOA_PUPDR &= ~( (0b11 << 2 ) |  (0b11 << 8 ));
*GPIOC_PUPDR &= ~( (0b11 << 0 ) |  (0b11 << 2 ) |  (0b11 << 6 ));
*GPIOA_PUPDR |=  ( (0b01 << 2 ) |  (0b01 << 8 ));
*GPIOC_PUPDR |=  ( (0b01 << 0 ) |  (0b01 << 2 ) |  (0b01 << 6 ));

}



void TIM2_Init(void)
{
    *APB1ENR |= (1 << 0);

    *GPIOA_MODER &= ~(0b11 << 0 );
    *GPIOA_MODER |=  (0b10 << 0 );

    *GPIOA_AFRL  &= ~(0b1111 << 0 );
    *GPIOA_AFRL  |=  (0b0001 << 0 );

    *TIM2_PSC =   15;
    *TIM2_ARR =   999;

   *TIM2_CCMR1 &= ~((1 << 3) | (0b111 << 4));
   *TIM2_CCMR1 |=  ((1 << 3) | (0b110 << 4));


   *TIM2_CCER &= ~(1 << 0);
   *TIM2_CCER |=  (1 << 0);

   *TIM2_EGR |=  (1 << 0);
    *TIM2_CR1    |= (1 << 0);
}

void Switch_Read(void)
{
    if (Switch_Debounce(GPIOC_IDR, 3))
    {
        *TIM2_CCR1 = 500;
    }

    else if (Switch_Debounce(GPIOC_IDR, 0))
    {
        *TIM2_CCR1 = 999;
    }

    else if (Switch_Debounce(GPIOC_IDR, 1))
    {
        *TIM2_CCR1 = 750;
    }

    else if (Switch_Debounce(GPIOA_IDR, 1))
    {
        *TIM2_CCR1 = 250;
    }

    else if (Switch_Debounce(GPIOA_IDR, 4))
    {
        *TIM2_CCR1 = 0;
    }

    else
    {
        *TIM2_CCR1 = 0;
    }
}



void Delay(volatile uint32_t count)
{
    for (volatile uint32_t i = 0; i < count; i++)
    {

    }
}



uint8_t Switch_Debounce(volatile uint32_t *IDR, uint32_t pin)
{
    if (!(*IDR & (1 << pin)))
    {
        Delay(10000);

        if (!(*IDR & (1 << pin)))
        {
            return 1;
        }
    }

    return 0;
}



int main(void)
{
    GPIO_Init();
    TIM2_Init();


    while (1)
    {
        Switch_Read();
    }
}
