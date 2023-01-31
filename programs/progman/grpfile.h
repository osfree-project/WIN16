struct tagGROUPHEADER {
  char  cIdentifier[4];	// PMCC for Win16 PMXX for Wine
  WORD  wCheckSum;
  WORD  cbGroup;		// Group file size
  WORD  nCmdShow;
  RECT  rcNormal;
  POINT ptMin;
  WORD  pName;
  WORD  wLogPixelsX;
  WORD  wLogPixelsY;
  BYTE  wBitsPerPixel;	// Error: as WORD in SDK
  BYTE  wPlanes;		// Error: as WORD in SDK
  WORD  wReserved;		// Must be 0
  WORD  cItems;
};
