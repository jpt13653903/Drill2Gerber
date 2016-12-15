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

int ConvertCoord(int Index){
 int  Value         = 0;
 int  Digits        = 0;
 int  PointPos      = 0;
 bool ExplicitPoint = false;

 int j;
 for(j = Index; Line[j]; j++){
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

 // Output is always with trailing zeros
 fprintf(Output, "%d", Value);

 if(!j){ // Prevent infinite loop
  printf("\nError while converting coordinate\n\n");
  RecognisedFormat = false;
  j++;
 }
 return j - Index;
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

void ConvertLine(){
 static bool Header = true;

 int j;
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
    j = 0;
    while(Line[j]){
     switch(Line[j]){
      case 'X':
      case 'Y':
      case '-': fprintf(Output, "%c", Line[j++]); break;
      case '+': j++;                              break;
      default : j += ConvertCoord(j);             break;
     }
    }
    fprintf(Output, "D03*\n");
    break;

   case 'M':
    if     (IsLine("M48")) Header = true;
    else if(IsLine("M30")) fprintf(Output, "M02*\n");
    break;

   default:
    break;
  }
 }
}
//------------------------------------------------------------------------------

int main(int argc, char** argv){
 if(argc < 2){
  printf(
   "Drill2Gerber, Version 1.0\n"
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
