#include "joystick.h"

#define ADC_SAMPLES 16
#define DEADZONE    80        // adjust depending on how sensitive your joystick is

static uint16_t centerX = 0;

/**
 * Initialize joystick handle
 * @param joystick Pointer to joystick handle
 * @param hadc Pointer to ADC handle
 * @param xChannel ADC channel for X-axis
 * @param yChannel ADC channel for Y-axis
 * @param buttonPort GPIO port for joystick button
 * @param buttonPin GPIO pin for joystick button
 */
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

/**
 * Read ADC channel and return averaged value
 * @param hadc Pointer to ADC handle
 * @param channel ADC channel to read
 */
static uint16_t Read_ADC_Channel(ADC_HandleTypeDef* hadc, uint32_t channel)
{
    ADC_ChannelConfTypeDef sConfig = {0};
    sConfig.Channel = channel;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_247CYCLES_5;
    HAL_ADC_ConfigChannel(hadc, &sConfig);

    uint32_t sum = 0;

    for (int i = 0; i < ADC_SAMPLES; i++)
    {
        HAL_ADC_Start(hadc);
        if (HAL_ADC_PollForConversion(hadc, 10) == HAL_OK)
            sum += HAL_ADC_GetValue(hadc);
        HAL_ADC_Stop(hadc);
    }

    return (uint16_t)(sum / ADC_SAMPLES);
}

/**
 * Calibration: find true center of joystick X at boot
 * @param joystick Pointer to joystick handle
 */
void Joystick_Calibrate(Joystick_HandleTypeDef *joystick)
{
    uint32_t sum = 0;

    for (int i = 0; i < 32; i++)
        sum += Read_ADC_Channel(joystick->hadc, joystick->xChannel);
        
    centerX = sum / 32;
}

/**
 * Return joystick direction (UP / DOWN / IDLE) with deadzone applied
 * @param joystick Pointer to joystick handle
 */
JoyStickDirection Joystick_GetDirection(Joystick_HandleTypeDef *joystick)
{
    uint16_t joystickX = Read_ADC_Channel(joystick->hadc, joystick->xChannel);

    int diff = (int)joystickX - (int)centerX;

    if (diff > DEADZONE)
        return JOY_UP;

    if (diff < -DEADZONE)
        return JOY_DOWN;

    return JOY_IDLE;
}

/**
 * Read X and Y axes (0–4095)
 * @param joystick Pointer to joystick handle
 * @param xy Array to store X and Y values
 */
void Joystick_ReadXY(Joystick_HandleTypeDef* joystick, uint16_t* xy)
{
    xy[0] = Read_ADC_Channel(joystick->hadc, joystick->xChannel); // X-axis
    xy[1] = Read_ADC_Channel(joystick->hadc, joystick->yChannel); // Y-axis
}

/**
 * Check if joystick button is pressed
 * @param joystick Pointer to joystick handle
 * @return 1 if pressed, 0 otherwise
 */
uint8_t Joystick_Pressed(Joystick_HandleTypeDef* joystick)
{
    // Check for Active Low and Debounce the button
    if (HAL_GPIO_ReadPin(joystick->buttonPort, joystick->buttonPin) == GPIO_PIN_RESET)
    {
        HAL_Delay(20);
        if(HAL_GPIO_ReadPin(joystick->buttonPort, joystick->buttonPin) == GPIO_PIN_RESET) 
        { return 1; }
    }
  
    return 0;
}