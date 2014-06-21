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
  );
  return 0;
 }

 Input = fopen(argv[1], "r");
 if(!Input){
  printf("Cannot open \"%s\" for reading\n", argv[1]);
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
  return 2;
 }

 char* Line = new char[0x1000];



 // Clean-up
 fclose(Input);
 fclose(Output);

 delete[] Line;
 delete[] OutputFile;

 return 0;
}
//------------------------------------------------------------------------------
