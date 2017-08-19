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

#include "main.h"
//------------------------------------------------------------------------------

void Pause(){
 printf("\nPress Enter to continue\n");
 getchar();
}
//------------------------------------------------------------------------------

bool ReadLine(){
 int c;
 int j = 0;

 c = fgetc(Input);
 if(c == EOF) return false;

 while(c != EOF){
  if(c == '\n'){
   Line[j] = 0;
   return true;
  }
  if(c != '\r') Line[j++] = c;
  c = fgetc(Input);
 }
 return true;
}
//------------------------------------------------------------------------------

bool IsLine(const char* String){
 int j;
 for(j = 0; Line[0] && String[j]; j++){
  if(Line[j] != String[j]) return false;
 }
 if(String[j]       ) return false; // Still characters left
 if(Line  [j] >= ' ') return false; // Still printable characters left
 return true;
}
//------------------------------------------------------------------------------

int GetTool(int* ToolChars){
 int j;
 int Tool = 0;

 for(j = 1; Line[j] >= '0' && Line[j] <= '9'; j++){
  Tool = 10*Tool + Line[j] - '0';
 }

 *ToolChars = j-1;
 return Tool;
}
//------------------------------------------------------------------------------

/*
The drill file format supports a maximum of 6 digits:

https://web.archive.org/web/20071030075236/http://www.excellon.com/
  manuals/program.htm                                                         */

int ConvertCoord(int Index, int* ValueOut){
 int  Value         = 0;
 int  Digits        = 0;
 int  PointPos      = 0;
 bool Sign          = false;
 bool ExplicitPoint = false;

 int j = Index;
 if(Line[j] == '+'){
  j++;
 }else if(Line[j] == '-'){
  Sign = true;
  j++;
 }

 while(Line[j]){
  if(Line[j] >= '0' && Line[j] <= '9'){
   Value = 10*Value + Line[j] - '0';
   Digits  ++;
   PointPos++;

  }else if(Line[j] == '.'){
   ExplicitPoint = true;
   PointPos      = 0;

  }else{
   break;
  }
  j++;
 }

 // Get the real value (scaled with 10^FractionDigits)
 if(ExplicitPoint){
  while(PointPos < FractionDigits){
   Value *= 10;
   Digits  ++;
   PointPos++;
  }

 // If leading zeros are specified, add the trailing ones
 }else if(LeadingZeros){
  while(Digits < (IntDigits + FractionDigits)){
   Value *= 10;
   Digits++;
  }
 }

 j -= Index;
 if(j == 0){ // Prevent infinite loop
  printf("\nError while converting coordinate\n\n");
  RecognisedFormat = false;
  j++;
 }

 // Output is always with trailing zeros
 *ValueOut = Sign ? -Value : Value;
 return j;
}
//------------------------------------------------------------------------------

void GetFormat(int Index){
 IntDigits = 0;
 while(Line[Index] == '0'){
  Index++;
  IntDigits++;
 }

 if(Line[Index] != '.') return;
 Index++;

 FractionDigits = 0;
 while(Line[Index] == '0'){
  Index++;
  FractionDigits++;
 }
}
//------------------------------------------------------------------------------

// Arcs are guaranteed to be less than 180 deg
void DoArc(double pX, double pY, double X, double Y, double R, bool CCW){
 double x = X - pX;
 double y = Y - pY;

 double e = CCW ? 1 : -1;
 double d = x*x + y*y;
 double h = R*R - d/4.0;

 if(h <= 0.0){ // Invalid radius
  x = (X - pX)/2.0;
  y = (Y - pY)/2.0;

 }else{
  d = sqrt(d);
  h = sqrt(h);

  x = (X - pX)/2.0 - e * h * ((Y - pY) / d);
  y = (Y - pY)/2.0 + e * h * ((X - pX) / d);
 }

 fprintf(Output,
  "X%dY%dI%dJ%dD01*\n",
  (int)round(X), (int)round(Y),
  (int)round(x), (int)round(y)
 );
}
//------------------------------------------------------------------------------

void DoCoord(int Index){
 // These are used for circular routing
 static int pX = 0, pY = 0, X = 0, Y = 0, I = 0, J = 0, R = 0;

 // Used to detect if this sets arc parameters, or includes a routing command
 bool ParameterOnly = false;

 while(Line[Index]){
  switch(Line[Index]){
   case 'X':
    ParameterOnly = false;
    Index ++;
    Index += ConvertCoord(Index, &X);
    break;

   case 'Y':
    ParameterOnly = false;
    Index ++;
    Index += ConvertCoord(Index, &Y);
    break;

   case 'I':
    if(Index == 0) ParameterOnly = true;
    Index ++;
    Index += ConvertCoord(Index, &I);
    R = round(sqrt((double)I*(double)I + (double)J*(double)J));
    break;

   case 'J':
    if(Index == 0) ParameterOnly = true;
    Index ++;
    Index += ConvertCoord(Index, &J);
    R = round(sqrt((double)I*(double)I + (double)J*(double)J));
    break;

   case 'A':
    if(Index == 0) ParameterOnly = true;
    Index ++;
    Index += ConvertCoord(Index, &R);
    break;

   default:
    break;
  }
 }

 if(ParameterOnly){
  pX = X;
  pY = Y;
  return;
 }

 switch(Mode){
  case Mode_Drill:
   if(pX != X) fprintf(Output, "X%d", X);
   if(pY != Y) fprintf(Output, "Y%d", Y);
   fprintf(Output, "D03*\n");
   break;

  case Mode_Route_Canned_CW:
  case Mode_Route_Canned_CCW:
   fprintf(Output, "X%dY%dD02*\n", X+R, Y);
   fprintf(Output, "I%dJ0D01*\n" ,  -R   );
   fprintf(Output, "X%dY%dD02*\n", X  , Y);
   break;

  default:
   if(Z_Axis == Z_Routing){
    switch(Mode){
     case Mode_Route_Move:
     case Mode_Route_Linear:
      if(pX != X) fprintf(Output, "X%d", X);
      if(pY != Y) fprintf(Output, "Y%d", Y);
      fprintf(Output, "D01*\n");
      break;

     case Mode_Route_CW:
      DoArc(pX, pY, X, Y, R, false);
      break;

     case Mode_Route_CCW:
      DoArc(pX, pY, X, Y, R, true);
      break;

     default:
      break;
    }
   }else{ // Move only
    if(pX != X) fprintf(Output, "X%d", X);
    if(pY != Y) fprintf(Output, "Y%d", Y);
    fprintf(Output, "D02*\n");
   }
   break;
 }

 pX = X;
 pY = Y;
}
//------------------------------------------------------------------------------

void ConvertLine(){
 static bool Header = true;

 int Tool;
 int CharCount;

 if(Header){
  switch(Line[0]){
   case 'I':
    if(Line[1] == 'N' && Line[2] == 'C' && Line[3] == 'H'){
     IntDigits        = 2;
     FractionDigits   = 4;
     LeadingZeros     = true;
     RecognisedFormat = true;

     if     (Line[5] == 'T') LeadingZeros = false;
     if     (Line[5] == '0') GetFormat(5);
     else if(Line[8] == '0') GetFormat(8);

     fprintf(Output,
      "%%FSLAX%d%dY%d%d*MOIN*%%\n",
      IntDigits, FractionDigits,
      IntDigits, FractionDigits
     );
    }
    break;

   case 'M':
    if(Line[1] == 'E' && Line[2] == 'T' && Line[3] == 'R'){
     IntDigits        = 3;
     FractionDigits   = 3;
     LeadingZeros     = true;
     RecognisedFormat = true;

     if     (Line[ 7] == 'T') LeadingZeros = false;
     if     (Line[ 7] == '0') GetFormat( 7);
     else if(Line[10] == '0') GetFormat(10);

     fprintf(Output,
      "%%FSLAX%d%dY%d%d*MOMM*%%\n",
      IntDigits, FractionDigits,
      IntDigits, FractionDigits
     );
    }
    break;

   case 'T': // Define drill width
    Tool = GetTool(&CharCount);
    fprintf(Output, "%%ADD%02dC,%s*%%\n", Tool+10, Line+CharCount+2);
    break;

   case '%':
    Header = false;
    fprintf(Output, "%%LPD*%%\nG01*\n");
    break;

   default:
    break;
  }
 }else{
  switch(Line[0]){
   case 'T':
    Tool = GetTool(&CharCount);
    if(Tool > 0) fprintf(Output, "D%02d*\n", Tool+10);
    break;

   case 'X':
   case 'Y':
   case 'I':
   case 'J':
   case 'A':
    DoCoord(0);
    break;

   case 'M':
    if     (IsLine("M48")) Header = true;
    else if(IsLine("M30")) fprintf(Output, "M02*\n");
    else if(IsLine("M15")) Z_Axis = Z_Routing;
    else if(IsLine("M16")) Z_Axis = Z_Retracted;
    else if(IsLine("M17")) Z_Axis = Z_Retracted;
    break;

   case 'G':
    if(
      (Line[1] == '0' && Line[2] == '5') |
      (Line[1] == '8' && Line[2] == '1')
    ){
      Z_Axis = Z_Retracted;
      Mode   = Mode_Drill;
      fprintf(Output, "G01*\n");

    }else if(Line[1] == '0' && Line[2] == '0'){
      Mode = Mode_Route_Move;
      DoCoord(3);

    }else if(Line[1] == '0' && Line[2] == '1'){
      Mode = Mode_Route_Linear;
      fprintf(Output, "G01*\n");
      DoCoord(3);

    }else if(Line[1] == '0' && Line[2] == '2'){
      Mode = Mode_Route_CW;
      fprintf(Output, "G02*\nG75*\n");
      DoCoord(3);

    }else if(Line[1] == '0' && Line[2] == '3'){
      Mode = Mode_Route_CCW;
      fprintf(Output, "G03*\nG75*\n");
      DoCoord(3);

    }else if(Line[1] == '3' && Line[2] == '2'){
      Mode = Mode_Route_Canned_CW;
      fprintf(Output, "G02*\nG75*\n");
      DoCoord(3);

    }else if(Line[1] == '3' && Line[2] == '3'){
      Mode = Mode_Route_Canned_CCW;
      fprintf(Output, "G03*\nG75*\n");
      DoCoord(3);

    }else if(Line[1] == '9' && Line[2] == '0'){
    }else if(Line[1] == '9' && Line[2] == '3'){
     printf("Warning: Zero-set command (G93) ignored\n");

    }else{
      printf(
       "Warning: Unsupported code: G%c%c\\n"
       "\n"
       "Please post a comment, with an example drill file, on\n"
       "https://sourceforge.net/p/gerber2pdf/discussion/bugs/\n",
       Line[1], Line[2]
      );
    }

   default:
    break;
  }
 }
}
//------------------------------------------------------------------------------

int main(int argc, char** argv){
 if(argc < 2){
  printf(
   "Drill2Gerber, Version 1.1\n"
   "Built on "__DATE__" at "__TIME__"\n"
   "\n"
   "Copyright (C) John-Philip Taylor\n"
   "jpt13653903@gmail.com\n"
   "\n"
   "This program is free software: you can redistribute it and/or modify\n"
   "it under the terms of the GNU General Public License as published by\n"
   "the Free Software Foundation, either version 3 of the License, or\n"
   "(at your option) any later version.\n"
   "\n"
   "This program is distributed in the hope that it will be useful,\n"
   "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
   "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
   "GNU General Public License for more details.\n"
   "\n"
   "You should have received a copy of the GNU General Public License\n"
   "along with this program.  If not, see <http://www.gnu.org/licenses/>\n"
   "\n"
   "Usage: Drill2Gerber input_file\n"
   "\n"
   "Tested on drill files from:\n"
   "- PCAD\n"
   "- KiCad\n"
   "- FreePCB\n"
   "- Microchip\n"
   "- Mentor Graphics\n"
   "- Autodesk Circuits\n"
  );
  Pause();
  return 0;
 }

 Input = fopen(argv[1], "r");
 if(!Input){
  printf("Cannot open \"%s\" for reading\n", argv[1]);
  Pause();
  return 1;
 }

 int j;
 for(j = 0; argv[1][j]; j++);
 char* OutputFile = new char[j+5];
 for(j = 0; argv[1][j]; j++) OutputFile[j] = argv[1][j];
 OutputFile[j++] = '.';
 OutputFile[j++] = 'g';
 OutputFile[j++] = 'r';
 OutputFile[j++] = 'b';
 OutputFile[j  ] =  0 ;

 Output = fopen(OutputFile, "w");
 if(!Output){
  printf("Cannot open \"%s\" for writing\n", OutputFile);
  fclose(Input);
  delete[] OutputFile;
  Pause();
  return 2;
 }

 Line = new char[0x1000];

 while(ReadLine()) ConvertLine();

 // Clean-up
 fclose(Input);
 fclose(Output);

 delete[] Line;
 delete[] OutputFile;

 if(!RecognisedFormat){
  printf(
   "Error: Unrecognised drill coordinate format\n"
   "\n"
   "Please post a comment, with an example drill file, on\n"
   "https://sourceforge.net/p/gerber2pdf/discussion/bugs/\n"
  );

 }else{
  printf("Drill to Gerber conversion successful\n");
 }

 return 0;
}
//------------------------------------------------------------------------------
