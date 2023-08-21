// ----------------------------------------
// Перекодировка текстового файла
// из OEM в ANSI
// ----------------------------------------

#define STRICT
#include <windows.h>
#include <commdlg.h>
#include <mem.h>

// Прототипы функций
HFILE GetSrcFile(void);
HFILE GetDstFile(void);
void  Oem2Ansi(HFILE, HFILE);

// -------------------------------
// Функция WinMain
// -------------------------------

#pragma argsused
int PASCAL
WinMain(HINSTANCE hInstance,
        HINSTANCE hPrevInstance,
        LPSTR     lpszCmdLine,
        int       nCmdShow)
{
  // Переменные для хранения
  // идентификаторов входного и выходного
  // файлов
  HFILE hfSrc, hfDst;

  // Открываем входной файл.
  // В случае ошибки или отказа от выбора
  // файла завершаем работу приложения
  hfSrc = GetSrcFile();
  if(!hfSrc) return 0;

  // Открываем выходной файл
  hfDst = GetDstFile();
  if(!hfDst) return 0;

  // Выполняем перекодировку файла
  Oem2Ansi(hfSrc, hfDst);

  // Закрываем входной и выходной файлы
  _lclose(hfSrc);
  _lclose(hfDst);

  return 0;
}

// -------------------------------
// Функция GetSrcFile
// Выбор файла для перекодировки
// -------------------------------

HFILE GetSrcFile(void)
{
  // Структура для выбора файла
  OPENFILENAME ofn;

  // Буфер для записи пути к выбранному файлу
  char szFile[256];

  // Буфер для записи имени выбранного файла
  char szFileTitle[256];

  // Фильтр расширений имени файлов
  char szFilter[256] =
         "Text Files\0*.txt;*.doc\0Any Files\0*.*\0";

  // Идентификатор открываемого файла
  HFILE hf;

  // Инициализация имени выбираемого файла
  // не нужна, поэтому создаем пустую строку
  szFile[0] = '\0';

  // Записываем нулевые значения во все поля
  // структуры, которая будет использована для
  // выбора файла
  memset(&ofn, 0, sizeof(OPENFILENAME));

  // Инициализируем нужные нам поля

  // Размер структуры
  ofn.lStructSize       = sizeof(OPENFILENAME);

  // Идентификатор окна
  ofn.hwndOwner         = NULL;

  // Адрес строки фильтра
  ofn.lpstrFilter       = szFilter;

  // Номер позиции выбора
  ofn.nFilterIndex      = 1;

  // Адрес буфера для записи пути
  // выбранного файла
  ofn.lpstrFile         = szFile;

  // Размер буфера для записи пути
  // выбранного файла
  ofn.nMaxFile          = sizeof(szFile);

  // Адрес буфера для записи имени
  // выбранного файла
  ofn.lpstrFileTitle    = szFileTitle;

  // Размер буфера для записи имени
  // выбранного файла
  ofn.nMaxFileTitle     = sizeof(szFileTitle);

  // В качестве начального каталога для
  // поиска выбираем текущий каталог
  ofn.lpstrInitialDir   = NULL;

  // Определяем режимы выбора файла
  ofn.Flags =   OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST
                                  | OFN_HIDEREADONLY;

  // Выбираем входной файл
  if (GetOpenFileName(&ofn)) {

    // Открываем выбранный файл
    hf = _lopen(ofn.lpstrFile, OF_READ);

    // Возвращаем идентификатор файла
    return hf;
  }
  // При отказе от выбора возвращаем
  // нулевое значение
  else return 0;
}

// -------------------------------
// Функция GetDstFile
// Выбор файла для записи
// результата перекодировки
// -------------------------------

HFILE GetDstFile(void)
{
  OPENFILENAME ofn;

  char szFile[256];
  char szFileTitle[256];
  char szFilter[256] =
         "Text Files\0*.txt;*.doc\0Any Files\0*.*\0";

  HFILE hf;

  szFile[0] = '\0';

  memset(&ofn, 0, sizeof(OPENFILENAME));

  ofn.lStructSize       = sizeof(OPENFILENAME);
  ofn.hwndOwner         = NULL;
  ofn.lpstrFilter       = szFilter;
  ofn.nFilterIndex      = 1;
  ofn.lpstrFile         = szFile;
  ofn.nMaxFile          = sizeof(szFile);
  ofn.lpstrFileTitle    = szFileTitle;
  ofn.nMaxFileTitle     = sizeof(szFileTitle);
  ofn.lpstrInitialDir   = NULL;
  ofn.Flags             = OFN_HIDEREADONLY;

  // Выбираем выходной файл
  if (GetSaveFileName(&ofn)) {

    // При необходимости создаем файл
    hf = _lcreat(ofn.lpstrFile, 0);
    return hf;
  }
  else return 0;
}

// -------------------------------
// Функция Oem2Ansi
// Перекодировка файла
// -------------------------------

void Oem2Ansi(HFILE hfSrcFile, HFILE hfDstFile)
{
  // Счетчик прочитанных байт
  int cbRead;

  // Буфер для считанных данных
  BYTE bBuf[2048];

  // Читаем в цикле файл и перекодируем его,
  // записывая результат в другой файл
  do {
    // Читаем в буфер 2048 байт из входного файла
    cbRead = _lread(hfSrcFile, bBuf, 2048);

    // Перекодируем содержимое буфера
    OemToAnsiBuff(bBuf, bBuf, cbRead);

    // Сохраняем содержимое буфера в
    // выходном файле
    _lwrite(hfDstFile, bBuf, cbRead);

  // Завершаем цикл по концу входного файла
  } while (cbRead != 0);
}