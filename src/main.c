#include <stdio.h>
#include "serverlist.h"
#include <unistd.h>
#include <termios.h>

int main(int argc, char const *argv[]) {
	struct termios tio;
	tcgetattr(STDIN_FILENO, &tio);
	tio.c_lflag &= ~(ICANON);
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &tio);
	
	char option;
	while (strcmp(&option,"2")) {
		printf("\033[H\033[J");

		printf("1 - Server List\n");
		printf("2 - Exit\n");
		printf("Input: ");

		scanf("%s", &option);
		
		if (!strcmp(&option, "1")) {
			serverlist_scene_init();
		}
	}

	return 0;
}