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

BOOL first_program = TRUE;

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
	SetTaskQueue(0, HWndDesktop.hQueue);

	// Plug the queue created for the desktop window with the
	// current task and expected Win version for this task.
	queuePtr = MK_FP( HWndDesktop, 0 );
	queuePtr->hTask = GetCurrentTask();
	queuePtr->ExpWinVersion = GetExeVersion();
  }

  TRACE("InitApp: returns WORD %d\n\r",1);
  return 1;
}

#endif

int WINAPI InitApp(HINSTANCE hInstance)
{
	int queueSize;

	FUNCTION_START

	/* Create task message queue */
	queueSize = GetProfileInt( "windows", "DefaultQueueSize", 8 );
	if (!SetMessageQueue( queueSize )) return 0;

	FUNCTION_END

	return 1;
}
