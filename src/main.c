#include "screen.h"
#include "keypad.h"
#include "game.h"

int main() {

	set_color(BRIGHT_WHITE, CYAN);
	clear_screen();

	int posR = 10, posC = 10;
	int row = 14, col = 39;

	intro();
	while(true)
	{
		movement(row,col,posR,posC);
	}

	return 0;
}
