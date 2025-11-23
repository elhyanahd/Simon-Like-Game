#ifndef __JOYSTICK_H
#define __JOYSTICK_H

#include "adc.h" 
#include "gpio.h"
#include "main.h"
#include "stm32wbxx_hal_gpio.h"

typedef struct 
{
    ADC_HandleTypeDef* hadc;   // ADC handle
    uint32_t xChannel;         // ADC channel for X-axis
    uint32_t yChannel;         // ADC channel for Y-axis
    GPIO_TypeDef* buttonPort;  // GPIO port for SW
    uint16_t buttonPin;        // GPIO pin for SW
} Joystick_HandleTypeDef;

// Initialize joystick handle
void Joystick_Init(Joystick_HandleTypeDef* joystick,
                   ADC_HandleTypeDef* hadc,
                   uint32_t xChannel,
                   uint32_t yChannel,
                   GPIO_TypeDef* buttonPort,
                   uint16_t buttonPin);

// Read X and Y axes (0–4095)
void Joystick_ReadXY(Joystick_HandleTypeDef* joystick, uint16_t* xy);

// Read SW button 
uint8_t Joystick_ReadButton(Joystick_HandleTypeDef* joystick);

#endif