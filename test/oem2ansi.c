// ----------------------------------------
// ������������� ���������� �����
// �� OEM � ANSI
// ----------------------------------------

#define STRICT
#include <windows.h>
#include <commdlg.h>
#include <mem.h>

// ��������� �������
HFILE GetSrcFile(void);
HFILE GetDstFile(void);
void  Oem2Ansi(HFILE, HFILE);

// -------------------------------
// ������� WinMain
// -------------------------------

#pragma argsused
int PASCAL
WinMain(HINSTANCE hInstance,
        HINSTANCE hPrevInstance,
        LPSTR     lpszCmdLine,
        int       nCmdShow)
{
  // ���������� ��� ��������
  // ��������������� �������� � ���������
  // ������
  HFILE hfSrc, hfDst;

  // ��������� ������� ����.
  // � ������ ������ ��� ������ �� ������
  // ����� ��������� ������ ����������
  hfSrc = GetSrcFile();
  if(!hfSrc) return 0;

  // ��������� �������� ����
  hfDst = GetDstFile();
  if(!hfDst) return 0;

  // ��������� ������������� �����
  Oem2Ansi(hfSrc, hfDst);

  // ��������� ������� � �������� �����
  _lclose(hfSrc);
  _lclose(hfDst);

  return 0;
}

// -------------------------------
// ������� GetSrcFile
// ����� ����� ��� �������������
// -------------------------------

HFILE GetSrcFile(void)
{
  // ��������� ��� ������ �����
  OPENFILENAME ofn;

  // ����� ��� ������ ���� � ���������� �����
  char szFile[256];

  // ����� ��� ������ ����� ���������� �����
  char szFileTitle[256];

  // ������ ���������� ����� ������
  char szFilter[256] =
         "Text Files\0*.txt;*.doc\0Any Files\0*.*\0";

  // ������������� ������������ �����
  HFILE hf;

  // ������������� ����� ����������� �����
  // �� �����, ������� ������� ������ ������
  szFile[0] = '\0';

  // ���������� ������� �������� �� ��� ����
  // ���������, ������� ����� ������������ ���
  // ������ �����
  memset(&ofn, 0, sizeof(OPENFILENAME));

  // �������������� ������ ��� ����

  // ������ ���������
  ofn.lStructSize       = sizeof(OPENFILENAME);

  // ������������� ����
  ofn.hwndOwner         = NULL;

  // ����� ������ �������
  ofn.lpstrFilter       = szFilter;

  // ����� ������� ������
  ofn.nFilterIndex      = 1;

  // ����� ������ ��� ������ ����
  // ���������� �����
  ofn.lpstrFile         = szFile;

  // ������ ������ ��� ������ ����
  // ���������� �����
  ofn.nMaxFile          = sizeof(szFile);

  // ����� ������ ��� ������ �����
  // ���������� �����
  ofn.lpstrFileTitle    = szFileTitle;

  // ������ ������ ��� ������ �����
  // ���������� �����
  ofn.nMaxFileTitle     = sizeof(szFileTitle);

  // � �������� ���������� �������� ���
  // ������ �������� ������� �������
  ofn.lpstrInitialDir   = NULL;

  // ���������� ������ ������ �����
  ofn.Flags =   OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST
                                  | OFN_HIDEREADONLY;

  // �������� ������� ����
  if (GetOpenFileName(&ofn)) {

    // ��������� ��������� ����
    hf = _lopen(ofn.lpstrFile, OF_READ);

    // ���������� ������������� �����
    return hf;
  }
  // ��� ������ �� ������ ����������
  // ������� ��������
  else return 0;
}

// -------------------------------
// ������� GetDstFile
// ����� ����� ��� ������
// ���������� �������������
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

  // �������� �������� ����
  if (GetSaveFileName(&ofn)) {

    // ��� ������������� ������� ����
    hf = _lcreat(ofn.lpstrFile, 0);
    return hf;
  }
  else return 0;
}

// -------------------------------
// ������� Oem2Ansi
// ������������� �����
// -------------------------------

void Oem2Ansi(HFILE hfSrcFile, HFILE hfDstFile)
{
  // ������� ����������� ����
  int cbRead;

  // ����� ��� ��������� ������
  BYTE bBuf[2048];

  // ������ � ����� ���� � ������������ ���,
  // ��������� ��������� � ������ ����
  do {
    // ������ � ����� 2048 ���� �� �������� �����
    cbRead = _lread(hfSrcFile, bBuf, 2048);

    // ������������ ���������� ������
    OemToAnsiBuff(bBuf, bBuf, cbRead);

    // ��������� ���������� ������ �
    // �������� �����
    _lwrite(hfDstFile, bBuf, cbRead);

  // ��������� ���� �� ����� �������� �����
  } while (cbRead != 0);
}