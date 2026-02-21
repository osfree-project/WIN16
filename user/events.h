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

// Modern version (@todo see latest constants for XButton etc.)
#define MOUSEEVENTF_MOVE        0x0001  //Movement occured
#define MOUSEEVENTF_LEFTDOWN    0x0002  //Button 1 (SW1, Left) changed to down
#define MOUSEEVENTF_LEFTUP      0x0004  //Button 1 (SW1, Left) changed to up
#define MOUSEEVENTF_RIGHTDOWN   0x0008  //Button 2 (SW3, Right) changed to down
#define MOUSEEVENTF_RIGHTUP     0x0010  //Button 2 (SW3, Right) changed to up
#define MOUSEEVENTF_MIDDLEDOWN  0x0020
#define MOUSEEVENTF_MIDDLEUP    0x0040
#define MOUSEEVENTF_WHEEL       0x0800
#define MOUSEEVENTF_ABSOLUTE    0x8000  //BX,CX are normalized absolute coords

// Old WIN 3.x constants (DDK)
#define SF_MOVEMENT	MOUSEEVENTF_MOVE
#define SF_B1_DOWN	MOUSEEVENTF_LEFTDOWN
#define SF_B1_UP	MOUSEEVENTF_LEFTUP
#define SF_B2_DOWN	MOUSEEVENTF_RIGHTDOWN
#define SF_B2_UP	MOUSEEVENTF_RIGHTUP
#define SF_ABSOLUTE	MOUSEEVENTF_ABSOLUTE

// Defined in early WINE
#define ME_MOVE		MOUSEEVENTF_MOVE
#define ME_LDOWN	MOUSEEVENTF_LEFTDOWN
#define ME_LUP		MOUSEEVENTF_LEFTUP
#define ME_RDOWN	MOUSEEVENTF_RIGHTDOWN
#define ME_RUP		MOUSEEVENTF_RIGHTUP
#define ME_ABSOLUTE	MOUSEEVENTF_ABSOLUTE
