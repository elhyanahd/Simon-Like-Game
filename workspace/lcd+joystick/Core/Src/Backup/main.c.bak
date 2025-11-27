/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "lcd1602.h"
#include <stdio.h> 
#include <string.h>
#include "joystick.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
typedef enum 
{
  WELCOME = 0,
  START,
  PLAYER_MENU,
  PLAYER_SELECT,
  ONE_PLAYER,
  TWO_PLAYERS,
  SCORES,
  GAME_RESULT,
  SLEEP
} GameState ; 

typedef struct 
{
  uint8_t numPlayers;
  uint8_t currentPlayer;
  uint8_t sequence[100];
  uint8_t sequenceLength;
  uint8_t playerInputs[2][100];
  uint8_t playerScores[2];
} GameInfo ;

Joystick_HandleTypeDef joystick;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/**
 * @brief  Display two lines of text on the LCD.
 * @param  lineOne: Pointer to the first line of text (max 16 characters).
 * @param  lineTwo: Pointer to the second line of text (max 16 characters
 */
void displayOnLCD(char* lineOne, char* lineTwo)
{
  LCD_GotoXY(0, 0);
  LCD_Print(lineOne);
  LCD_GotoXY(0, 1);
  LCD_Print(lineTwo);
  HAL_Delay(1000);
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* Configure the peripherals common clocks */
  PeriphCommonClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ADC1_Init();
  MX_USART1_UART_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start(&htim2);
  /*** Initialize LCD ***/
  LCD_Init();
  LCD_Cls();
  /*** End of LCD Initialization ***/

  /*** Initialize Joystick ***/
  Joystick_Init(&joystick, &hadc1, ADC_CHANNEL_7, ADC_CHANNEL_8, 
                 GPIOC, GPIO_PIN_6);
  Joystick_Calibrate(&joystick);
  /*** End of Joystick Initialization ***/

  GameState currentState = WELCOME;
  GameInfo play;
  char lineOne[17] = {0}, lineTwo[17] = {0};
  int directionDelay = 0;
  JoyStickDirection direction = JOY_IDLE, lastDirection = JOY_IDLE;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    // FSM to manage game states
    switch(currentState)
    {
      // Display WELCOME message and 
      //Push to start prompt after 2 seconds
      case WELCOME:
        LCD_Cls();
        snprintf(lineOne, sizeof(lineOne), "Welcome to the");
        snprintf(lineTwo, sizeof(lineTwo), "Simon Game");
        displayOnLCD(lineOne, lineTwo);
        HAL_Delay(2000);  
        
        LCD_Cls();
        snprintf(lineOne, sizeof(lineOne), "Push To Start!");
        snprintf(lineTwo, sizeof(lineTwo), "");
        displayOnLCD(lineOne, lineTwo);
        currentState = START;
        break;

      // Check if Joystick is pressed to start the game
      case START:
        if (Joystick_Pressed(&joystick)) 
        {  currentState = PLAYER_MENU;  }        
        break;

      // Display Player selection menu
      case PLAYER_MENU:
        LCD_Cls();
        snprintf(lineOne, sizeof(lineOne), "1 player OR");
        snprintf(lineTwo, sizeof(lineTwo), "2 players?");
        displayOnLCD(lineOne, lineTwo);
        currentState = PLAYER_SELECT;
        break;

      // Handle Player selection based on joystick UP/DOWN input
      case PLAYER_SELECT:
        // Read the joystick input to determine the number of players
        direction = Joystick_GetDirection(&joystick);

        // Adding "debounce" logic to help resolve jittery input
        //causing direciton UP to be seen once
        if(directionDelay < 3 && lastDirection == JOY_UP && direction == JOY_IDLE)
        {
          direction = JOY_UP;
          directionDelay++;
        }
        else 
        { directionDelay = 0; }

        // Update display only if joystick direction has changed
        if (direction != JOY_IDLE && direction != lastDirection)
        {
          
          if (direction == JOY_UP)
          {
            LCD_Cls();
            snprintf(lineOne, sizeof(lineOne), "<1> player OR");
            snprintf(lineTwo, sizeof(lineTwo), "2 players?");
            displayOnLCD(lineOne, lineTwo);
            play.numPlayers = 1;
            direction = JOY_UP;
          }    
          else if (direction == JOY_DOWN) 
          {
            LCD_Cls();
            snprintf(lineOne, sizeof(lineOne), "1 player OR");
            snprintf(lineTwo, sizeof(lineTwo), "<2> players?");
            displayOnLCD(lineOne, lineTwo);
            play.numPlayers = 2;
          }

          lastDirection = direction;  // store last direction
        }   

        // Check if joystick is pressed to confirm selection
        // Move to the state which matches the mode selected
        if (Joystick_Pressed(&joystick)) 
        {
          if (play.numPlayers == 1)
            currentState = ONE_PLAYER;
          else 
            currentState = TWO_PLAYERS;
        }
        break;
      
        // For LCD+Joystick testing purposes only,
        // Display the mode type selected, wait 2 seconds then
        // return to the PLAYER SELECT state for further 
        // joystick to LCD interaction testing
      case ONE_PLAYER:
        LCD_Cls();
        snprintf(lineOne, sizeof(lineOne), "1 Player Mode");
        snprintf(lineTwo, sizeof(lineTwo), "");
        displayOnLCD(lineOne, lineTwo);
        HAL_Delay(2000);  //wait 2 seconds
        currentState = PLAYER_MENU;
        break;

      case TWO_PLAYERS:
        LCD_Cls();
        snprintf(lineOne, sizeof(lineOne), "2 Player Mode");
        snprintf(lineTwo, sizeof(lineTwo), "");
        displayOnLCD(lineOne, lineTwo);
        HAL_Delay(2000);  //wait 2 seconds
        currentState = PLAYER_MENU;
        break;
      // case SCORES:
      //   // Handle scores display logic
      //   break;
      // case GAME_RESULT:
      //   // Handle game result logic
      //   break;
      default:
        currentState = START;
        break;
    }
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Macro to configure the PLL multiplication factor
  */
  __HAL_RCC_PLL_PLLM_CONFIG(RCC_PLLM_DIV2);

  /** Macro to configure the PLL clock source
  */
  __HAL_RCC_PLL_PLLSOURCE_CONFIG(RCC_PLLSOURCE_MSI);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_10;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the SYSCLKSource, HCLK, PCLK1 and PCLK2 clocks dividers
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK4|RCC_CLOCKTYPE_HCLK2
                              |RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.AHBCLK2Divider = RCC_SYSCLK_DIV2;
  RCC_ClkInitStruct.AHBCLK4Divider = RCC_SYSCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief Peripherals Common Clock Configuration
  * @retval None
  */
void PeriphCommonClock_Config(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Initializes the peripherals clock
  */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SMPS;
  PeriphClkInitStruct.SmpsClockSelection = RCC_SMPSCLKSOURCE_HSI;
  PeriphClkInitStruct.SmpsDivSelection = RCC_SMPSCLKDIV_RANGE0;

  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN Smps */

  /* USER CODE END Smps */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
