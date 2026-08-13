/* MAKEHELP.C – Build INFOPLUS.HLP from PAGE_xx.INF files (translated from makehelp.pas) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VERSION     157L
#define FIRSTFILE   0
#define LASTFILE    21
#define TABLE_SIZE  64          /* количество элементов в таблице смещений */

int main(void)
{
    FILE *bytefile;
    FILE *infile, *outfile;
    char inname[20], outname[] = "INFOPLUS.HLP";
    char s[256];
    unsigned long thetable[TABLE_SIZE];
    unsigned filecount;
    unsigned long file_size;

    printf("Making INFOPLUS.HLP for version %lu\n", VERSION);
    printf("Creating pages %d through %d\n", FIRSTFILE, LASTFILE);

    /* Инициализация таблицы нулями */
    memset(thetable, 0, sizeof(thetable));

    /* Удаляем старый файл, если существует */
    remove(outname);

    for (filecount = FIRSTFILE; filecount <= LASTFILE; filecount++)
    {
        /* Определяем текущий размер выходного файла */
        bytefile = fopen(outname, "rb");
        if (bytefile == NULL)
        {
            file_size = 256;    /* если файла нет, начинаем со смещения 256 */
        }
        else
        {
            fseek(bytefile, 0, SEEK_END);
            file_size = ftell(bytefile);
            fclose(bytefile);
        }

        printf("%u-%lu\n", filecount, file_size);

        /* Читаем текущую таблицу (если файл уже существует) */
        bytefile = fopen(outname, "rb+");
        if (bytefile == NULL)
        {
            /* Файл не существует – создаём и записываем пустую таблицу */
            bytefile = fopen(outname, "wb");
            if (bytefile == NULL)
            {
                fprintf(stderr, "Unable to create %s!\n", outname);
                return 1;
            }
            /* Записываем нулевую таблицу */
            fwrite(thetable, sizeof(unsigned long), TABLE_SIZE, bytefile);
            fclose(bytefile);
            bytefile = fopen(outname, "rb+");
        }

        /* Читаем существующую таблицу */
        rewind(bytefile);
        fread(thetable, sizeof(unsigned long), TABLE_SIZE, bytefile);

        /* Обновляем версию и смещение для данной страницы */
        thetable[63] = VERSION;
        thetable[filecount] = file_size;

        /* Записываем обновлённую таблицу обратно */
        rewind(bytefile);
        fwrite(thetable, sizeof(unsigned long), TABLE_SIZE, bytefile);
        fclose(bytefile);

        /* Формируем имя входного файла */
        sprintf(inname, "PAGE_%02u.INF", filecount);

        /* Открываем входной файл */
        infile = fopen(inname, "rt");
        if (infile == NULL)
        {
            fprintf(stderr, "Unable to open %s!!\a\n", inname);
            return 1;
        }

        /* Открываем выходной файл для добавления */
        outfile = fopen(outname, "at");
        if (outfile == NULL)
        {
            fprintf(stderr, "Unable to open %s for appending!\n", outname);
            fclose(infile);
            return 1;
        }

        /* Копируем строки */
        while (fgets(s, sizeof(s), infile) != NULL)
        {
            /* Убираем символ новой строки (fgets сохраняет \n) */
            s[strcspn(s, "\r\n")] = '\0';
            fprintf(outfile, "%s\n", s);   /* в текстовом режиме \n -> \r\n */
        }
        /* Завершающий маркер */
        fprintf(outfile, "$END\n");

        fclose(outfile);
        fclose(infile);
    }

    printf("INFOPLUS.HLP successfully created.\n");
    return 0;
}
