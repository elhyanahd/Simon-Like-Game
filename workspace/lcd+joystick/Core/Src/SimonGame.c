#include "SimonGame.h"
#include "lcd1602.h"
#include "gpio.h"
#include "tim.h"
#include <stdio.h> 
#include <string.h>
#include <stdlib.h>

static int buttonTimer = 0;
static int buttonPressed = 0;
static char lineOne[17] = {0}, lineTwo[17] = {0};

/**
 * @brief  Initialize the game state and information.
 * @param  game: Pointer to the Game structure to initialize.
 */
void Game_Init(Game* game)
{
    game->state = WELCOME;
    memset(&game->info, 0, sizeof(GameInfo));
}

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

/**
 * @brief Helper function created for debouncing the switch buttons.
 *        If the given GPIO switch is pressed (active-low state) wait
 *        the .2 s delay time and check if switch is released.
 * 
 * @param GPIO 
 * @param Pin 
 * @return int (1 - if switch is pressed and released, else 0)
 */
int debounceButton(GPIO_TypeDef *GPIO, uint16_t Pin)
{
  if(HAL_GPIO_ReadPin(GPIO, Pin) == GPIO_PIN_RESET)
  {
    HAL_Delay(20);
    if(HAL_GPIO_ReadPin(GPIO, Pin) == GPIO_PIN_RESET) 
    { return 1; }
  }
  
  return 0;
}

/**
 * @brief  Compare the player's input sequence with the computer's sequence.
 * @param  game: Pointer to the Game structure.
 */
void compareSequences(Game* game)
{
  if (game->info.numPlayers == 1)
  {
    for(int i = 0; i < game->info.sequenceLength; i++)
    {
      if(game->info.sequence[i] != game->info.playerInputs[0][i])
      {
        game->state = GAME_RESULT;
        return;
      }
    }
  }
  else
  {
    for(int i = 0; i < game->info.sequenceLength - 1; i++)
    {
      if(game->info.playerInputs[0][i] != game->info.playerInputs[1][i])
      {
        game->state = GAME_RESULT;
        return;
      }
    }
  }
}

/**
 * @brief  Run the main game loop, managing game states and transitions.
 * @param  game: Pointer to the Game structure.
 * @param  joystick: Pointer to the Joystick handle structure.
 */
void Game_Run(Game* game, Joystick_HandleTypeDef* joystick)
{
    static int directionDelay = 0;
    static JoyStickDirection direction = JOY_IDLE, lastDirection = JOY_IDLE;
    
    // FSM to manage game states
    switch(game->state)
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
        game->state = START;

        buttonTimer = 0; 
        __HAL_TIM_SET_COUNTER(&htim2, 0);
        break;

      // Check if Joystick is pressed to start the game
      //Wait 10 seconds and if no input return to SLEEP state
      case START:
        while (buttonTimer < 100) // 10 seconds timeout
        {
          if (Joystick_Pressed(joystick)) 
          {  
            game->state = PLAYER_MENU;  
            break; 
          }  
        }  
        
        if (game->state != PLAYER_MENU)
        { game->state = SLEEP; }
        break;

      // Display Player selection menu
      case PLAYER_MENU:
        LCD_Cls();
        snprintf(lineOne, sizeof(lineOne), "1 player OR");
        snprintf(lineTwo, sizeof(lineTwo), "2 players?");
        displayOnLCD(lineOne, lineTwo);
        game->state = PLAYER_SELECT;
        break;

      // Handle Player selection based on joystick UP/DOWN input
      case PLAYER_SELECT:
        // Read the joystick input to determine the number of players
        direction = Joystick_GetDirection(joystick);

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
            game->info.numPlayers = 1;
            direction = JOY_UP;
          }    
          else if (direction == JOY_DOWN) 
          {
            LCD_Cls();
            snprintf(lineOne, sizeof(lineOne), "1 player OR");
            snprintf(lineTwo, sizeof(lineTwo), "<2> players?");
            displayOnLCD(lineOne, lineTwo);
            game->info.numPlayers = 2;
          }

          lastDirection = direction;  // store last direction
        }   

        // Check if joystick is pressed to confirm selection
        // Move to the state which matches the mode selected
        if (Joystick_Pressed(joystick)) 
        {
          if (game->info.numPlayers == 1)
          {
            game->state = ONE_PLAYER;

            // Seed the random number generator using joystick readings
            uint16_t seed[2] = {0};
            Joystick_ReadXY(joystick, seed);
            srand(seed[0] ^ seed[1]);
          }
          else 
            game->state = TWO_PLAYERS;

          game->info.round = 1;
          game->info.sequenceLength = 1;
          game->info.sequenceSpeed = 1000; // Initial speed 1 s
        }
        break;
      
        // For LCD+Joystick testing purposes only,
        // Display the mode type selected, wait 2 seconds then
        // return to the PLAYER SELECT state for further 
        // joystick to LCD interaction testing
      case ONE_PLAYER:
        LCD_Cls();
        snprintf(lineOne, sizeof(lineOne), "Round %d", game->info.round);
        snprintf(lineTwo, sizeof(lineTwo), "Simon's Turn!");
        displayOnLCD(lineOne, lineTwo);
        HAL_Delay(500);  //wait 1/2 second
        computerTurn(game);

        LCD_Cls();
        snprintf(lineOne, sizeof(lineOne), "Player's Turn!");
        snprintf(lineTwo, sizeof(lineTwo), "Score: %d", game->info.playerScores[0]);
        displayOnLCD(lineOne, lineTwo);
        HAL_Delay(500);  //wait 1/2 second
        playerTurn(game, 1);

        if(game->state != GAME_RESULT)
        {
          compareSequences(game);
          if(game->state != GAME_RESULT)
          {
            // Prepare for next round
            //Increase round, sequence length and speed
            game->info.round++;
            game->info.sequenceLength++;
            if(game->info.sequenceSpeed > 400)
            { game->info.sequenceSpeed -= 5; } // Increase speed
          }
        }
        break;

      case TWO_PLAYERS:
        LCD_Cls();
        snprintf(lineOne, sizeof(lineOne), "Round %d", game->info.round);
        snprintf(lineTwo, sizeof(lineTwo), "");
        displayOnLCD(lineOne, lineTwo);
        HAL_Delay(500);  //wait 1/2 second
        
        snprintf(lineOne, sizeof(lineOne), "Player 1's Turn");
        snprintf(lineTwo, sizeof(lineTwo), "Score: %d", game->info.playerScores[0]);
        displayOnLCD(lineOne, lineTwo);
        HAL_Delay(500);  //wait 1/2 second
        playerTurn(game, 1);

        if (game->state != GAME_RESULT)
        {
          if(game->info.round > 1)
          {
            compareSequences(game);
            if(game->state != GAME_RESULT)
            { game->info.playerScores[0]++; }
          }
               
          // Player 2's turn
          game->info.sequenceLength++;
          LCD_Cls();
          snprintf(lineOne, sizeof(lineOne), "Player 2's Turn");
          snprintf(lineTwo, sizeof(lineTwo), "Score: %d", game->info.playerScores[1]);
          displayOnLCD(lineOne, lineTwo);
          HAL_Delay(500);  //wait 1/2 second
          playerTurn(game, 2);

          if(game->state != GAME_RESULT)
          {
            compareSequences(game);
            if(game->state != GAME_RESULT)
            {
              // Prepare for next round
              game->info.round++;
              game->info.sequenceLength++;
            }
          }
        }
        break;

      case GAME_RESULT:
        LCD_Cls();
        if (game->info.numPlayers == 1 )
        {
          snprintf(lineOne, sizeof(lineOne), "Game Over!");
          snprintf(lineTwo, sizeof(lineTwo), "Score: %d", game->info.playerScores[0]);
          displayOnLCD(lineOne, lineTwo);
          HAL_Delay(2000);  //wait 2 seconds
        }
        else
        {
          snprintf(lineOne, sizeof(lineOne), "Game Over!");
          if(game->info.playerScores[0] > game->info.playerScores[1])
          {
            snprintf(lineTwo, sizeof(lineTwo), "Player 1 Wins");
          }
          else if(game->info.playerScores[0] < game->info.playerScores[1])
          {
            snprintf(lineTwo, sizeof(lineTwo), "Player 2 Wins");
          }
          else
          {
            snprintf(lineTwo, sizeof(lineTwo), "Players Tied");
          }
          displayOnLCD(lineOne, lineTwo);
          HAL_Delay(2000);  //wait 1 second

          snprintf(lineOne, sizeof(lineOne), "P1 Score: %d", game->info.playerScores[0]);
          snprintf(lineTwo, sizeof(lineTwo), "P2 Score: %d", game->info.playerScores[1]);
          displayOnLCD(lineOne, lineTwo);
          HAL_Delay(2000);  //wait 2 seconds
        }
        game->state = PLAYER_MENU;
        break;
      
      case PLAY_AGAIN:
        LCD_Cls();
        snprintf(lineOne, sizeof(lineOne), "Play Again?");
        snprintf(lineTwo, sizeof(lineTwo), "Push to Start");
        displayOnLCD(lineOne, lineTwo);
        game->state = START;
        break;

      case SLEEP:
        LCD_Cls();
        snprintf(lineOne, sizeof(lineOne), "");
        snprintf(lineTwo, sizeof(lineTwo), "");
        displayOnLCD(lineOne, lineTwo);

        // Wait for joystick press to wake up
        if (Joystick_Pressed(joystick)) 
        { game->state = WELCOME;  } 
        break;

      default:
        game->state = SLEEP;
        break;
    }
}


/**
 * @brief  Handle the computer's turn in the game.
 * @param  game: Pointer to the Game structure.
 */
void computerTurn(Game* game)
{
  for(int i = 0; i < game->info.sequenceLength; i++)
  {
    // select random numbers for the color sequence
    // 0 - Red, 1 - Blue, 2 - Yellow, 3 - Green
    int colorRandom = rand() % 4;

    // Store the color code in the sequence array
    game->info.sequence[i] = colorRandom;
  }

  for(int i = 0; i < game->info.sequenceLength; i++)
  {
    // Light up the corresponding LED based on the color code
    switch(game->info.sequence[i])
    {
      case 0: // Red
        HAL_GPIO_WritePin(LEDR_GPIO_Port, LEDR_Pin, GPIO_PIN_SET);
        HAL_Delay(game->info.sequenceSpeed);
        HAL_GPIO_WritePin(LEDR_GPIO_Port, LEDR_Pin, GPIO_PIN_RESET);
        break;
      case 1: // Blue
        HAL_GPIO_WritePin(LEDB_GPIO_Port, LEDB_Pin, GPIO_PIN_SET);
        HAL_Delay(game->info.sequenceSpeed);
        HAL_GPIO_WritePin(LEDB_GPIO_Port, LEDB_Pin, GPIO_PIN_RESET);
        break;
      case 2: // Yellow
        HAL_GPIO_WritePin(LEDY_GPIO_Port, LEDY_Pin, GPIO_PIN_SET);
        HAL_Delay(game->info.sequenceSpeed);
        HAL_GPIO_WritePin(LEDY_GPIO_Port, LEDY_Pin, GPIO_PIN_RESET);
        break;
      case 3: // Green
        HAL_GPIO_WritePin(LEDG_GPIO_Port, LEDG_Pin, GPIO_PIN_SET);
        HAL_Delay(game->info.sequenceSpeed);
        HAL_GPIO_WritePin(LEDG_GPIO_Port, LEDG_Pin, GPIO_PIN_RESET);
        break;
      default:
        break;
    }
  }
}

/**
 * @brief  Handle the player's turn in the game.
 * @param  game: Pointer to the Game structure.
 * @param  player: The current player number (1 or 2).
 */
void playerTurn(Game* game, uint8_t player)
{

  for(int i = 0; i < game->info.sequenceLength; i++)
  {
    // Reset and start timer for player's input timeout
    buttonTimer = 0;    
    buttonPressed = 0;
    __HAL_TIM_SET_COUNTER(&htim2, 0);
    
    while (buttonTimer < 50 && buttonPressed == 0) // 5 seconds timeout
    {
      if(debounceButton(RedButton_GPIO_Port, RedButton_Pin))
      { 
        game->info.playerInputs[player - 1][i] = 0;
        buttonPressed = 1;
        break; 
      }
      
      if(debounceButton(BlueButton_GPIO_Port, BlueButton_Pin))
      { 
        game->info.playerInputs[player - 1][i] = 1;
        buttonPressed = 1;
        break; 
      }
          
      if(debounceButton(YellowButtonm_GPIO_Port, YellowButtonm_Pin))
      { 
        game->info.playerInputs[player - 1][i] = 2;
        buttonPressed = 1;
        break; 
      }
      
      if(debounceButton(GreenButton_GPIO_Port, GreenButton_Pin))
      { 
        game->info.playerInputs[player - 1][i] = 3;
        buttonPressed = 1;
        break; 
      }
    }
    
    if(buttonPressed == 0)
    {
      game->state = GAME_RESULT;
      return;
    }
  }
}

/**
 * @brief  Timer period elapsed callback.
 * @param  htim: Pointer to a TIM_HandleTypeDef structure that contains
 *                the configuration information for TIM module.
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2)
    { buttonTimer++;  }
}