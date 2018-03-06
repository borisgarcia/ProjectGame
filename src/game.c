#include "game.h"

void intro()
{
  keypad_init();
  bool a = true;
  while(a)
  {
    uint8_t key = keypad_getkey();
    set_cursor(14,29);
  	puts("WELCOME TO SNAKE GAME!");
  	set_cursor(16,28);
  	puts("PRESS SPACE TO CONTINUE!");
    if(key == 8)
    {
      set_color(BRIGHT_WHITE, BLACK);
      clear_screen();
      a = false;
    }
  }
}

void movement(int row, int col, int col_f,int row_f)
{
  int d = 75;
  //set_color(BRIGHT_WHITE, BLACK);
  keypad_init();
  uint8_t key = keypad_getkey();
  while(true)
  {
    key = keypad_getkey();
    if(key == 1)
    {
      if(col >= 1 && col <= 80)
      {
        //clear_screen();
        delay_ms(d);
        col = col - 1;
        clear_screen();
      }
    }

    if(key == 2)
    {
      if(col >= 0 && col <= 78)
      {
        //clear_screen();
        delay_ms(d);
        col = col + 1;
        clear_screen();
      }
    }

    if(key == 3)
    {
      if(row >= 0 && row < 29)
      {
        //clear_screen();
        delay_ms(d);
        row = row + 1;
        clear_screen();
      }
    }

    if(key == 4)
    {
      if(row > 0 && row < 30)
      {
        //clear_screen();
        delay_ms(d);
        row = row - 1;
        clear_screen();
      }
    }

    if(row_f == row && col_f == col)
    {
      clear_screen();
      row_f = randRows()-1;
      col_f = randCols()-1;
    }
    drawSnake(row,col);
    drawFood(row_f,col_f);
  }
}

void drawSnake(int r,int c)
{
  set_cursor(r, c);
  puts("O");
}

void drawFood(int r_f,int c_f)
{
  set_cursor(r_f, c_f);
  puts("X");
}

uint8_t randCols()
{
    static uint8_t nSeed = 5;
    nSeed = (8253729 * nSeed + 2396403);
    return (nSeed  % 79);
}

uint8_t randRows()
{
    static uint8_t nSeed = 5;
    nSeed = (8253729 * nSeed + 2396403);
    return (nSeed  % 29);
}

void GameOver()
{
  set_color(BRIGHT_WHITE, LIGHT_BLUE);
  clear_screen();
  keypad_init();
  bool a = true;
  while(a)
  {
    uint8_t key = keypad_getkey();
    set_cursor(14,29);
  	puts("Game Over!");
  	set_cursor(16,21);
  	puts("PRESS SPACE TO Start Again!");
    if(key == 8)
    {
      set_color(BRIGHT_WHITE, BLACK);
      clear_screen();
      a = false;
    }
  }
}
