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

void ConvertLine(){
 static bool Header = true;

 int Tool;
 int ToolChars;

 if(Header){
  switch(Line[0]){
   case 'I':
    if(IsLine("INCH,00.0000")){
     RecognisedFormat = true;
     fprintf(Output, "%%FSLAX24Y24*MOIN*%%\n");

    }else if(IsLine("INCH,LZ")){
     RecognisedFormat = true;
     fprintf(Output, "%%FSTAX24Y24*MOIN*%%\n");
    }
    break;

   case 'M':
    if(IsLine("METRIC,TZ,000.000")){
     RecognisedFormat = true;
     fprintf(Output, "%%FSLAX33Y33*MOMM*%%\n");
    }
    break;

   case 'T': // Define drill width
    Tool = GetTool(&ToolChars);
    fprintf(Output, "%%ADD%02dC,%s*%%\n", Tool+10, Line+ToolChars+2);
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
    Tool = GetTool(&ToolChars);
    if(Tool > 0) fprintf(Output, "D%02d*\n", Tool+10);
    break;

   case 'X':
   case 'Y':
    for(Tool = 0; Line[Tool]; Tool++){
     if(Line[Tool] != '+') fprintf(Output, "%c", Line[Tool]);
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
