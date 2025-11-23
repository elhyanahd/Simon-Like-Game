#include "main.h"
#include "gpio.h"
#include "tim.h"

#define	pin_E	    GPIO_PIN_1
#define	pin_RS	    GPIO_PIN_0
#define pin_DB4     GPIO_PIN_0
#define pin_DB5     GPIO_PIN_1
#define pin_DB6     GPIO_PIN_2
#define pin_DB7	    GPIO_PIN_3
#define Bit_RESET   GPIO_PIN_RESET
#define Bit_SET     GPIO_PIN_SET

/*
 *   Declare Functions
 */
extern void Delay_ms(uint8_t);
extern void Delay_us(uint8_t);
extern void LCD_Init(void);
extern void LCD_cmd_4bit(uint8_t cmd);
extern void LCD_data_4bit(uint8_t data);
extern void LCD_Cls(void);
extern void LCD_GotoXY(int column, int line);
extern void LCD_Print(char *string);
extern void LCD_PrintI(uint32_t num);
extern void LCD_PrintH(uint32_t num);
extern void LCD_PrintB8(uint8_t num);
extern void LCD_PrintB16(uint16_t num);
