#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <string.h>
#include <time.h>

#define FORESEE	2

int main(int argc, char *argv[])
{
	int i, j, c, in, done, charcount, errors, time_total;
	time_t time_start, time_end;
	char string[80];
	char **buf;
	double cpm, wpm;
	FILE *fd;
	struct termios termios_old, termios_new;

	char *filename = argv[1];
	if (strcmp("-", filename) == 1) fd = stdin;
	else fd = fopen(filename, "r");
	if (fd == NULL) {
		printf("\nWoops, couldn't read file.\n");
		return 1;
	}

	tcgetattr(0, &termios_old);
	termios_new = termios_old;
	termios_new.c_lflag &= (~ICANON & ~ECHO);
	tcsetattr(0, TCSANOW, &termios_new);

	done = 0;
	charcount = 0;
	time_start = time(NULL);
	errors = 0;

	buf = malloc(120);

	for (i = 0; fgets(string, 120, fd) != NULL; ++i)
	{
		buf[i] = malloc(strlen(string) + 1);
		strcpy(buf[i], string); 
		/* printf("%d:	%s", i, buf[i]); */
	}
	
	buf[i] = NULL;
	i = 0;

	printf("\e[2J\e[0;0H");
	while (!done)
	{
		if (buf[i] == NULL) {
			done = 1;
			time_end = time(NULL);
			break;
		}

		for (j = 0; j < 1 + FORESEE && buf[i+j] != NULL; ++j)
			printf("\e[0;%dm%s", j == 0 ? 0 : 37, buf[i+j]);
		printf("\e[%dA", j);

		c = 0;	
		while ( buf[i][c] != '\0') {
			in = getchar();
			if (in == '\e') {
				done = 1;
				time_end = time(NULL);
				break;
			}
			if (in != buf[i][c]) {
				printf("\e[37;7m");
				++errors;
			}
			else printf("\e[0;38;5;8m");

			putchar(buf[i][c]);
			++c;
			++charcount;
		}
		++i;
	}
	time_total = (int)difftime(time_end, time_start);

	cpm = (60*(charcount - errors) / (time_total));
	wpm = (12*(charcount - errors) / (time_total));

	printf("\e[0m");
	printf(	"\nResults:\n\n"
		"	Time:		%d\n"
		"	Chars typed:	%d\n"
		"	Errors:		%d (~%d\%)\n"
		"	CPM:		%0.f (accounting for errors, roughly)\n"
		"	WPM:		%0.f (accounting for errors, roughly)\n"
		"\n",
		time_total, charcount, errors, (int)(errors * 100 / charcount),
	     	cpm, wpm );

	tcsetattr(0, TCSANOW, &termios_old);

	return 0;
}

