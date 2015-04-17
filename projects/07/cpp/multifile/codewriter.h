#ifndef VM_CODE_WRITER
#define VM_CODE_WRITER

#include <iostream>
#include <string.h>
#include <fstream>
#include <sstream>
#include "codewriter.h"
#include "vm_cmd_type.h" //defines enumerated types for command types of virtual machine
#include <vector>
#include <algorithm>


class codewriter{
private:
  std::fstream outfile;
  std::string newfilename;
  int symcount;

public:  
  codewriter(std::string filename){
    
    //std::cout << "Code Writer constructor called with file name" << filename << std::endl;
    outfile.open(filename.c_str(), std::ios::out);
    if(!outfile)
      std::cerr << "CODE WRITER: Error Opening Output file [" << filename.c_str() << "] \n";
    symcount =0;
  }
  
  std::string getNextSymbolName(){
    std::ostringstream symbol("symbol");
    symbol <<"symbol" <<symcount++;
    return symbol.str();
  }

  
  void setFileName(std::string str) {
    newfilename = str;
  }
  std::string incrementSP(){
    return(" @SP\r\n M=M+1\r\n");
  }
  std::string decrementSP(){
    return(" @SP\r\n M=M-1\r\n");
  }
  std::string pushDtoStack(){
    return(" @SP\r\n A=M\r\n M=D\r\n" + incrementSP());
  }
  std::string popDfromStack(){
    return(decrementSP() + " @SP\r\n A=M\r\n D=M\r\n");
  }
  std::string pushTruetoStack(){
    return(" D=-1\r\n" + pushDtoStack());
  }
  std::string pushFalsetoStack(){
    return(" D=0\r\n" + pushDtoStack());
  }

  int writeArithmetic(std::string str){
    //std::cout<< "Wrote Arithmetic Command:" << str << std::endl;
    if((str == "add") || (str == "sub") || (str == "and") || str == "or"){ //process commands with binary stack arguments.
      outfile << popDfromStack(); //got the first argument and put in register D
      outfile << decrementSP(); //getting the second argument
      outfile << " @SP\r\n A=M\r\n"; // get ready to access memory pointed to by SP
      if(str == "add")
	outfile << " M=D+M\r\n";
      if(str == "sub")
	outfile << " M=M-D\r\n";
      if(str == "and")
	outfile << " M=D&M\r\n";
      if(str == "or")
	outfile << " M=D|M\r\n";
      outfile << incrementSP(); //prepare stack for next command
      return(1);
    } // endif( str == add, sub, and, or)
    
    
    if((str =="eq") || (str == "gt") || (str == "lt")){
      std::string symbol1 = getNextSymbolName(); //get two symbols to manage branching.
      std::string symbol2 = getNextSymbolName();
      outfile << popDfromStack();
      outfile << decrementSP();
      outfile << " @SP\r\n A=M\r\n D=M-D\r\n @" << symbol1 << "\r\n";
      // D now contains result of x-y and jump location has been set in register A using [symbol1].
      //in case result of x-y means "True" result for command, jump to [symbol1] now. Else don't jump.
      if(str == "eq")
	outfile << " D;JEQ\r\n";  // jump if x == y 
      if(str == "gt")
	outfile << " D;JGT\r\n";  //jump if x > y  
      if(str == "lt")
	outfile << " D;JLT\r\n";  //jump if x < y
      // Now write code for pushing False to Stack. because if the jump did not occur we need to push False to stack
      // After false has been pushed to stack, jump unconditionally to [symbol2]. This is necessary for skipping the code 
      //that will push true after [symbol1] is encountered.
      
      outfile << pushFalsetoStack()<< " @" << symbol2 <<"\r\n 0;JMP\r\n"; 
      outfile << "(" << symbol1 << ")\r\n"; //write definition of [symbol1]. The contorl will jump here if a True needs to be pushed to stack.
      outfile << pushTruetoStack();
      //now insert definition of [symbol2]. The next command will write commands after that.
      outfile << "(" << symbol2 << ")\r\n";
      return(1);
    }//endif(str == eq, gt, lt)
    if(str == "neg"){
      outfile << decrementSP(); // make SP point to last pushed element.
      outfile <<"@SP\r\n A=M\r\n M=-M\r\n";
      outfile << incrementSP();
      return(1);
    } // endif(str == neg)
    if(str == "not"){
      outfile << decrementSP();
      outfile << "@SP\r\n A=M\r\n M=!M\r\n";
      outfile << incrementSP();
      return(1);
    }//endif (str == not)
    
    // Unknown command
    std::cerr << "Code Writer Error: Received unknown arithmetic command [" << str <<"] \n";
    return(0);
  }//end writeArithmetic
  void writePush(vm_command_type ct, std::string segment, int index){
    if(segment == "constant"){
      outfile << " @" << index << "\r\n D=A\r\n"; //constant loaded into register D
      outfile << pushDtoStack();
      }

  }//end of writePush
  void writePop(vm_command_type ct, std::string segment, int index){
    if(segment == "constant"){
      decrementSP(); // there is no need to overwrite top of stack as it will be overwritten by next push command.
    }

  }//end of writePush
 

  ~codewriter(){
    // Write an indefinite loop at the end.
    outfile << "(END)\r\n @END\r\n 0;JMP\r\n";

    outfile.close(); //release resources
  }  


}; // End of class codewriter

#endif
