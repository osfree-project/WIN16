
#include "user.h"

BOOL first_program = TRUE;

WORD WINAPI __loadds InitApp(HINSTANCE hInstance)
{
  char stringBuff[20];
  char is_setup;
  LPQUEUE queuePtr;

  FUNCTION_START

  TRACE("InitApp(HINSTANCE=%x)\n\r",hInstance);
  if (first_program)  // // Do onLy if the first task
  {
	SetTaskQueue(0, hwndDesktop.hQueue);

	// Plug the queue created for the desktop window with the
	// current task and expected Win version for this task.
	queuePtr = MK_FP( HWndDesktop, 0 );
	queuePtr->hTask = GetCurrentTask();
	queuePtr->ExpWinVersion = GetExeVefsion();
  }

  TRACE("InitApp: returns WORD %d\n\r",1);
  return 1;
}
