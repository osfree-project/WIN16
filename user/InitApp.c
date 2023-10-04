#include <windows.h>

#include "user.h"

BOOL first_program = TRUE;

WORD WINAPI __loadds InitApp(HINSTANCE hInstance)
{
  char stringBuff[20];
  char is_setup;
  LPQUEUE queuePtr;

	FUNCTION_START

//    APISTR((LF_APICALL,"InitApp(HINSTANCE=%x)\n",hInstance));
  if (first_program)
  {
//    SetTaskQueue(0, hwndDesktop.hQueue);
  }

//    APISTR((LF_APIRET,"InitApp: returns WORD %d\n",1));
    return 1;
}
