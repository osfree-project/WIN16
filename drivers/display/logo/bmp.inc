BITMAPFILEHEADER STRUCT
  bfType        WORD      ?
  bfSize        DWORD      ?
  bfReserved1   WORD      ?
  bfReserved2   WORD      ?
  bfOffBits     DWORD      ?
BITMAPFILEHEADER ENDS
 
BITMAPINFOHEADER STRUCT
  biSize            DWORD      ?
  biWidth           DWORD      ?
  biHeight          DWORD      ?
  biPlanes          WORD       ?
  biBitCount        WORD       ?
  biCompression     DWORD      ?
  biSizeImage       DWORD      ?
  biXPelsPerMeter   DWORD      ?
  biYPelsPerMeter   DWORD      ?
  biClrUsed         DWORD      ?
  biClrImportant    DWORD      ?
BITMAPINFOHEADER ENDS
