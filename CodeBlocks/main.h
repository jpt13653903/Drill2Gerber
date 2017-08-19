//==============================================================================
// Copyright (C) John-Philip Taylor
// jpt13653903@gmail.com
//
// This file is part of Drill2Gerber
//
// This file is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>
//==============================================================================

#ifndef main_h
#define main_h
//------------------------------------------------------------------------------

#include <math.h>
#include <stdio.h>
//------------------------------------------------------------------------------

FILE* Input;
FILE* Output;

char* Line;

int  IntDigits        = 3;
int  FractionDigits   = 3;
bool LeadingZeros     = true;
bool RecognisedFormat = false;

enum MODE{
 Mode_Drill,
 Mode_Route_Move,
 Mode_Route_Linear,
 Mode_Route_CW,
 Mode_Route_CCW,
 Mode_Route_Canned_CW,
 Mode_Route_Canned_CCW,
} Mode = Mode_Drill;

enum Z_AXIS{
  Z_Routing,
  Z_Retracted
} Z_Axis = Z_Retracted;
//------------------------------------------------------------------------------

#endif
//------------------------------------------------------------------------------
