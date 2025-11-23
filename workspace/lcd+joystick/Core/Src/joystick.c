#include "joystick.h"

void Joystick_Init(Joystick_HandleTypeDef* joystick,
                   ADC_HandleTypeDef* hadc,
                   uint32_t xChannel,
                   uint32_t yChannel,
                   GPIO_TypeDef* buttonPort,
                   uint16_t buttonPin)
{
    joystick->hadc = hadc;
    joystick->xChannel = xChannel;
    joystick->yChannel = yChannel;
    joystick->buttonPort = buttonPort;
    joystick->buttonPin = buttonPin;
}

// Helper function to read a single ADC channel
static uint16_t Read_ADC_Channel(ADC_HandleTypeDef* hadc, uint32_t channel)
{
    ADC_ChannelConfTypeDef sConfig = {0};
    sConfig.Channel = channel;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_247CYCLES_5;
    HAL_ADC_ConfigChannel(hadc, &sConfig);

    HAL_ADC_Start(hadc);
    uint16_t value = 0;
    if(HAL_ADC_PollForConversion(hadc, 10) == HAL_OK)
        value = HAL_ADC_GetValue(hadc);
    HAL_ADC_Stop(hadc);

    return value;
}

// Read X and Y axes
void Joystick_ReadXY(Joystick_HandleTypeDef* joystick, uint16_t* xy)
{
    xy[0] = Read_ADC_Channel(joystick->hadc, joystick->xChannel); // X-axis
    xy[1] = Read_ADC_Channel(joystick->hadc, joystick->yChannel); // Y-axis
}

uint8_t Joystick_ReadButton(Joystick_HandleTypeDef* joystick)
{
    return HAL_GPIO_ReadPin(joystick->buttonPort, joystick->buttonPin) == GPIO_PIN_RESET; // return 1 if pressed
}