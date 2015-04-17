
#include <iostream>
#include <string.h>
#include <fstream>
#include <sstream>
#include "vm_cmd_type.h" //defines enumerated types for command types of virtual machine
#include "codewriter.h"
#include "vmparser.h"
#include <vector>
#include <algorithm>
//using namespace std;

int main(int argc, char **argv){
  
  
  //std::cout << "Hello World \n";
  std::string in = "test.vm";
  std::string out ="Xxx.asm";
  std::string  line;
  int lineno;

  vm_command_type command_type;
  if(argc >1)
    in.assign(argv[1]); // get input filename from command line argument.
  if(argc >2)
    out.assign(argv[2]);

  vmparser parser(in);
  codewriter writer(out);



  while(parser.hasMoreCommands()){
    parser.advance();
    parser.getCurrentString(line);
    lineno = parser.getLineNumber();
    //std::cout<< "LINE=" << parser.getLineNumber() << ":" << line <<std::endl;
    command_type = parser.commandType();
    //std::cout << "COMMAND TYPE =" << enumlist[command_type] << "ARG1=" << parser.arg1() << "    ARG2=" << parser.arg2()<< std::endl; 
     
    switch(command_type){
    case C_EMPTY: case C_COMMENT:
      break; // skip comments and empty lines.
    case C_ARITHMETIC:
      writer.writeArithmetic(parser.arg1());
      break;
    case C_PUSH:
      writer.writePush(command_type, parser.arg2(), parser.getPushPopIndex());
      break;
    case C_POP:
      writer.writePop(command_type, parser.arg2(), parser.getPushPopIndex());
      break;
    case C_INVALID:
      std::cerr << "LINE No. " << lineno << " ERROR: The command [" << line << "] is not in the VM specification " << std::endl;
      break;
    default:
      std::cerr<<"LINE NO. "<< lineno <<" WARNING: The command ["<< line << "] is not implemented." << std::endl;
      break;
    }
  }
}
