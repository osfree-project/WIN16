/* msd.c Ц главный модуль (альтернатива main.c) */
#include <stdio.h>
#include <stdlib.h>
#include "msd.h"

void init(int argc, char *argv[]);   /* прототипы уже в msd.h, но можно оставить */
void runit(int argc, char *argv[]);

int main(int argc, char *argv[])
{
    init(argc, argv);
    runit(argc, argv);
    return 0;
}
