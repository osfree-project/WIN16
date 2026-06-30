/*
 * tools.h – Prototypes for all drawing tools (C89, Win16)
 */

#ifndef TOOLS_H
#define TOOLS_H

#include <windows.h>
#include "canvas.h"
#include "palette.h"

void Tool_PenDown(int x, int y, BOOL left);
void Tool_PenMove(int x, int y, BOOL left);
void Tool_PenUp(int x, int y, BOOL left);

void Tool_LineDown(int x, int y, BOOL left);
void Tool_LineMove(int x, int y, BOOL left);
void Tool_LineUp(int x, int y, BOOL left);

void Tool_RectDown(int x, int y, BOOL left);
void Tool_RectMove(int x, int y, BOOL left);
void Tool_RectUp(int x, int y, BOOL left);

void Tool_EllipseDown(int x, int y, BOOL left);
void Tool_EllipseMove(int x, int y, BOOL left);
void Tool_EllipseUp(int x, int y, BOOL left);

void Tool_FillDown(int x, int y, BOOL left);
void Tool_FillMove(int x, int y, BOOL left);
void Tool_FillUp(int x, int y, BOOL left);

void Tool_EraserDown(int x, int y, BOOL left);
void Tool_EraserMove(int x, int y, BOOL left);
void Tool_EraserUp(int x, int y, BOOL left);

void Tool_ColorPickDown(int x, int y, BOOL left);
void Tool_ColorPickMove(int x, int y, BOOL left);
void Tool_ColorPickUp(int x, int y, BOOL left);

void Tool_SelectDown(int x, int y, BOOL left);
void Tool_SelectMove(int x, int y, BOOL left);
void Tool_SelectUp(int x, int y, BOOL left);

void Tool_FreeSelDown(int x, int y, BOOL left);
void Tool_FreeSelMove(int x, int y, BOOL left);
void Tool_FreeSelUp(int x, int y, BOOL left);

void Tool_ZoomDown(int x, int y, BOOL left);
void Tool_ZoomMove(int x, int y, BOOL left);
void Tool_ZoomUp(int x, int y, BOOL left);

void Tool_BrushDown(int x, int y, BOOL left);
void Tool_BrushMove(int x, int y, BOOL left);
void Tool_BrushUp(int x, int y, BOOL left);

void Tool_AirBrushDown(int x, int y, BOOL left);
void Tool_AirBrushMove(int x, int y, BOOL left);
void Tool_AirBrushUp(int x, int y, BOOL left);

void Tool_TextDown(int x, int y, BOOL left);
void Tool_TextMove(int x, int y, BOOL left);
void Tool_TextUp(int x, int y, BOOL left);

void Tool_BezierDown(int x, int y, BOOL left);
void Tool_BezierMove(int x, int y, BOOL left);
void Tool_BezierUp(int x, int y, BOOL left);

void Tool_ShapeDown(int x, int y, BOOL left);
void Tool_ShapeMove(int x, int y, BOOL left);
void Tool_ShapeUp(int x, int y, BOOL left);

void Tool_RRectDown(int x, int y, BOOL left);
void Tool_RRectMove(int x, int y, BOOL left);
void Tool_RRectUp(int x, int y, BOOL left);

#endif
