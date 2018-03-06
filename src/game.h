#ifndef GAME_H_
#define GAME_H_

#include "screen.h"
#include "keypad.h"

void movement(int row, int col, int col_f,int row_f);
void intro();
void GameOver();
void drawSnake(int r,int c);
void drawFood(int r_f, int c_f);
uint8_t randRows();
uint8_t randCols();
#endif
