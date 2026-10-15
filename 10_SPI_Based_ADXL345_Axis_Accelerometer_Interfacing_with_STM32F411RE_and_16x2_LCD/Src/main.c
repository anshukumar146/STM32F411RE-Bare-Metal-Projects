#include <stdint.h>

#define RCC     0x40023800
#define GPIOA   0x40020000
#define GPIOB   0x40020400
#define GPIOC   0x40020800
#define SPI1    0x40013000

#define RCC_AHB1ENR   ((volatile uint32_t *)(RCC   + 0x30))
#define RCC_APB2ENR   ((volatile uint32_t *)(RCC   + 0x44))

#define GPIOA_MODER ((volatile uint32_t *)(GPIOA + 0x00))
#define GPIOA_ODR   ((volatile uint32_t *)(GPIOA + 0x14))

#define GPIOB_MODER ((volatile uint32_t *)(GPIOB + 0x00))
#define GPIOB_AFRL  ((volatile uint32_t *)(GPIOB + 0x20))

#define GPIOC_MODER ((volatile uint32_t *)(GPIOC + 0x00))
#define GPIOC_ODR   ((volatile uint32_t *)(GPIOC + 0x14))

#define SPI1_CR1   ((volatile uint32_t *)(SPI1 + 0x00))
#define SPI1_SR    ((volatile uint32_t *)(SPI1 + 0x08))
#define SPI1_DR    ((volatile uint32_t *)(SPI1 + 0x0C))



void GPIO_Init(void);
void SPI1_Init(void);
uint8_t SPI1_TransferByte(uint8_t value);
void CS_Low(void);
void CS_High(void);
void SPI1_WaitBusy(void);

void delay(uint32_t n);
void LCD_SendNibble(uint8_t nibble);
void LCD_Command(uint8_t command);
void LCD_Init(void);
void LCD_Data(uint8_t data);
void LCD_SetCursor(uint8_t row, uint8_t column);
void LCD_PrintString(const char *string);

uint8_t ADXL345_ReadCommand(uint8_t address);
uint8_t ADXL345_ReadRegister(uint8_t address);
void ADXL345_WriteRegister(uint8_t address, uint8_t value);
int16_t ADXL345_ReadAxis(uint8_t address);
void LED_Status(uint8_t sensor_ok);
int16_t Accel_ToHundredths(int16_t raw);
void LCD_PrintAcceleration(int16_t value);


void CS_Low(void)
{
    *GPIOC_ODR &= ~(0b1 << 2);
}
void CS_High(void)
{
    *GPIOC_ODR |= (0b1 << 2);
}

void SPI1_WaitBusy(void)
{
    while(((*SPI1_SR >> 7) & 1))
    {

    }
}

void GPIO_Init(void)
{
    *RCC_AHB1ENR |= ((1 << 0)|(1 << 1)|(1 << 2));


    *GPIOA_MODER &= ~((0b11 << 10)|
                    (0b11 << 12)|
                    (0b11 << 16)|
                    (0b11 << 18)|
                    (0b11 << 22)|
                    (0b11 << 24));


    *GPIOA_MODER |=  ((0b01 << 10)|
                    (0b01 << 12)|
                    (0b01 << 16)|
                    (0b01 << 18)|
                    (0b01 << 22)|
                    (0b01 << 24));

    *GPIOB_MODER &= ~((0b11 << 6)|
                    (0b11 << 8)|
                    (0b11 << 10));



    *GPIOB_MODER |=  ((0b10 << 6)|
                    (0b10 << 8)|
                    (0b10 << 10));

    *GPIOB_AFRL &= ~((0b1111 << 12)|
                    (0b1111 << 16)|
                    (0b1111 << 20));

    *GPIOB_AFRL |= ((0b0101 << 12)|
                    (0b0101 << 16)|
                    (0b0101 << 20));

    *GPIOC_MODER &= ~((0b11 << 4)|(0b11 << 8)|(0b11 << 10));
    *GPIOC_MODER |=  ((0b01 << 4)|(0b01 << 8)|(0b01 << 10));

    *GPIOC_ODR |= (0b1 << 2); //SET PC2 AS HIGH as cs is low active.
}

void SPI1_Init(void)
{
    *RCC_APB2ENR |= (1 << 12);

    *SPI1_CR1 &= ~(
                    (0b1 << 0)|
                    (0b1 << 1)|
                    (0b1 << 2)|
                    (0b111 << 3)|
                    (0b1 << 6)|
                    (0b1 << 7)|
                    (0b1 << 8)|
                    (0b1 << 9)
                );

    *SPI1_CR1 |= (
                    (0b1 << 2)|
                    (0b010 << 3)|
                    (0b1 << 0)|
                    (0b1 << 1)|
                    (0b1 << 9)|
                    (0b1 << 8)|
                    (0b1 << 6)

                );

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

    *GPIOA_ODR &= ~((1 << 5)|(1 << 6)|(1 << 11)|(1 << 12));
    *GPIOA_ODR |= ((D4 << 5)|(D5 << 6)|(D6 << 11)|(D7 << 12));


    *GPIOA_ODR |=  (0b1 << 9);
    delay(5000);

    *GPIOA_ODR &= ~(0b1 << 9);
    delay(5000);
}


void delay(uint32_t n)
{
    for (volatile uint32_t i = 0; i < n; i++)
    {
    }
}
void LCD_Command(uint8_t command)
{
    *GPIOA_ODR &= ~(0b1 << 8);

    uint8_t high_nibble = ((command >> 4) & 0x0F);
    LCD_SendNibble(high_nibble);

    uint8_t low_nibble = ((command >> 0) & 0x0F);
    LCD_SendNibble(low_nibble);

    delay(1000);
}

void LCD_Init(void)
{
    delay(100000);

    *GPIOA_ODR &= ~(1 << 8);

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

void LCD_Data(uint8_t data)
{
    *GPIOA_ODR |= (0b1 << 8);

    uint8_t high_nibble = ((data >> 4) & 0x0F);
    LCD_SendNibble(high_nibble);

    uint8_t low_nibble = ((data >> 0) & 0x0F);
    LCD_SendNibble(low_nibble);

    delay(2000);
}

void LCD_SetCursor(uint8_t row, uint8_t column)
{
    uint8_t address;

    if(row == 1)
    {
        address = 0x80;
    }
    else
    {
        address = 0xC0;
    }

    address += column - 1;

    LCD_Command(address);
}

void LCD_PrintString(const char *string)
{
    while(*string != '\0')
    {
        LCD_Data(*string);
        string++ ;
    }
}

uint8_t SPI1_TransferByte(uint8_t value)
{
    uint8_t data;

    while (!((*SPI1_SR >> 1) & 1))
    {

    }

    *SPI1_DR = value;

    while (!((*SPI1_SR >> 0) & 1))
    {

    }
    data = *SPI1_DR;

    return(data);
}

uint8_t ADXL345_ReadCommand(uint8_t address)
{
    uint8_t command = address | ( 1 << 7 );

    return(command);
}

uint8_t ADXL345_ReadRegister(uint8_t address)
{
    CS_Low();

    uint8_t comm = ADXL345_ReadCommand(address);

    SPI1_TransferByte(comm);

    uint8_t register_data = SPI1_TransferByte(0x00);

    SPI1_WaitBusy();

    CS_High();

    return(register_data);
}

void ADXL345_WriteRegister(uint8_t address, uint8_t value)
{
    CS_Low();
    SPI1_TransferByte(address);
    SPI1_TransferByte(value);
    SPI1_WaitBusy();
    CS_High();
}

int16_t ADXL345_ReadAxis(uint8_t address)
{
    uint8_t low_byte = ADXL345_ReadRegister(address);
    uint8_t high_byte = ADXL345_ReadRegister((address + 1));

    uint16_t raw = (high_byte << 8) | low_byte;
    return((int16_t)raw);
}

void LED_Status(uint8_t sensor_ok)
{
        if(sensor_ok == 1)
    {
        *GPIOC_ODR |= (1 << 4);
        *GPIOC_ODR &= ~(1 << 5);

    }
    else
    {
        *GPIOC_ODR |= (1 << 5);
        *GPIOC_ODR &= ~(1 << 4);
    }
}

int16_t Accel_ToHundredths(int16_t raw)
{
    int32_t raw1 = raw * 100;

    int16_t raw2 = raw1 / 256 ;
    return(raw2);
}

void LCD_PrintAcceleration(int16_t value)
{
 int32_t magnitude;
 int32_t temp;
 uint8_t  sign;
 uint16_t whole ;
 uint16_t fraction  ;
 uint8_t tenth , ones ;

 temp = value;

 if(value >= 0)
 {
    sign = '+';
 }
 else
 {
    sign = '-';
 }

 if(temp >= 0)
 {
    magnitude = temp ;
 }
 else
 {
    magnitude = (-1*temp);
 }

 whole = (magnitude / 100);
 fraction = (magnitude % 100);

 tenth = (fraction / 10);
 ones = (fraction % 10);


 LCD_Data(sign);
 LCD_Data((whole + '0'));
 LCD_Data('.');
 LCD_Data((tenth + '0'));
 LCD_Data((ones + '0'));

}

int main(void)
{
    GPIO_Init();
    SPI1_Init();
    LCD_Init();

    uint8_t device_id = ADXL345_ReadRegister(0x00);

    uint8_t sensor_ok = 0;

    if (device_id == 0xE5)
    {
    sensor_ok = 1;

    LED_Status(sensor_ok);

    ADXL345_WriteRegister(0x31,((1 << 3) |(0b00 << 0)));
    ADXL345_WriteRegister(0x2D, (1 << 3));


  while(1)
    {
    int16_t X = ADXL345_ReadAxis(0x32);
    int16_t Y = ADXL345_ReadAxis(0x34);
    int16_t Z = ADXL345_ReadAxis(0x36);
    int16_t x_hundredth;
    int16_t y_hundredth;
    int16_t z_hundredth;


        x_hundredth = Accel_ToHundredths(X);
        LCD_SetCursor(1 ,1);
        LCD_Data('X');
        LCD_Data(':');
        LCD_PrintAcceleration(x_hundredth);


        y_hundredth = Accel_ToHundredths(Y);
        LCD_SetCursor(1 ,9);
        LCD_Data('Y');
        LCD_Data(':');
        LCD_PrintAcceleration(y_hundredth);

        z_hundredth = Accel_ToHundredths(Z);
        LCD_SetCursor(2 ,1);
        LCD_Data('Z');
        LCD_Data(':');
        LCD_PrintAcceleration(z_hundredth);

         delay(100000);

        }
    }
    else
    {
        sensor_ok = 0;
         LED_Status(sensor_ok);

          while (1)
        {
                            //  sensor  not detected
        }
    }

    return 0;

}
