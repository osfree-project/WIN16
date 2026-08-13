/*

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, see
<https://www.gnu.org/licenses/>.

*/

#include "user.h"

#if 0
WORD WINAPI InitApp(HINSTANCE hInstance)
{
  char stringBuff[20];
  char is_setup;
  LPQUEUE queuePtr;

  FUNCTION_START

  TRACE("InitApp(HINSTANCE=%x)\n\r",hInstance);
  if (first_program)  // // Do only if the first task
  {
	SetTaskQueue(0, hwndDesktop.hQueue);

	// Plug the queue created for the desktop window with the
	// current task and expected Win version for this task.
	queuePtr = MAKELP(hwndDesktop, 0);
	queuePtr->hTask = GetCurrentTask();
	queuePtr->ExpWinVersion = GetExeVersion();
  }

  TRACE("InitApp: returns WORD %d\n\r",1);
  return 1;
}

#endif

// @todo finish it!!!
// Windows Internals p.278
// Undocumented Windows p.379, p.465
int WINAPI InitApp(HINSTANCE hInstance)
{
	WND * pWnd;
	LPQUEUE lpQueue;

	FUNCTION_START

 	// if first app then relink queue to it. For now it disabled because Wine based code uses slightly another approach for
	// desktop creation. @todo relink doesnt wotk because desktop window doesn't have a queue in Wine
	if (0 & fFirstProgram)
	{
		pWnd = WIN_FindWndPtr(hwndDesktop);
		SetTaskQueue(0, pWnd->hmemTaskQ);

		// Plug the queue created for the desktop window with the
		// current task and expected Win version for this task.
		lpQueue = MAKELP(pWnd->hmemTaskQ, 0);
		lpQueue->OwningTask = GetCurrentTask();
		lpQueue->ExpWinVersion = GetExeVersion();
	}
	else //create new queue
        {
		/* Create task message queue */
		if (!SetMessageQueue( DefQueueSize )) return 0;
	}

//	SetTaskSignalProc(0, (FARPROC)SignalProc);

	// Set divide by zero interrupt handler
	//SetDivZero();

	if (fFirstProgram)
	{
		fFirstProgram = FALSE;
	}
	
	FUNCTION_END

	return 1;
}
