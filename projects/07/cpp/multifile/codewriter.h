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
  std::string vmfilename;
  int symcount;
  int stackbase, heapbase, pointerbase, tempbase;
 
public:  
  codewriter(std::string filename){
    
    //std::cout << "Code Writer constructor called with file name" << filename << std::endl;
    outfile.open(filename.c_str(), std::ios::out);
    if(!outfile)
      std::cerr << "CODE WRITER: Error Opening Output file [" << filename.c_str() << "] \n";
    symcount =0;stackbase=256; heapbase=2048; pointerbase=3; tempbase=5;
    //writeinit();
  }
  
  
  void writeinit(){//sets up the memory segments in registers at the begining.
    
    outfile << " @"<<stackbase<<"\r\n D=A\r\n @SP\r\n M=D\r\n"; 
    // put stackbase=256 in SP. Stack starts at RAM address 256.
    // The ARG, LCL, THIS, THAT segments dependant on the function that is being called.
    // Here we initilize those assuming that we are in main() function.
    outfile <<" @"<< heapbase << "\r\n D=A\r\n @ARG\r\n M=D\r\n";
    // Segment argument for arguments is initially set to the heap base. We will set aside 256 RAM
    //addresses for argument segment. Base addres of argument segment is saved in the ARG register.
    outfile <<" @"<< (heapbase + 256) << "\r\n D=A\r\n @LCL\r\n M=D\r\n";
    //Segment local will be mapped to heapbase+256, where argument segment ends. It will be assigned
    //1024 memory addresses. Its base address will be stored in register LCL.
    outfile <<" @"<< (heapbase + 256 + 1024) << "\r\n D=A\r\n @THIS\r\n M=D\r\n";
    // Segment this starts where segment local ends. It will be assigned 1024
    // memory addresses and its address will be stored in THIS register. 
    outfile <<" @"<< (heapbase + 256 + 1024 + 1024) << "\r\n D=A\r\n @THAT\r\n M=D\r\n";
    // Segment that starts where semgemnt this ends. Since we don't need any other segment in heap, segmant that can grow
    // uptill the end of heap. its base address will be stored in THAT register.
    // Initialization is complete.
    // Note that the Segments [static, pointer, temp] do not need initilization. As there base addresses are not stored in RAM
    // but are taken from the specification. They are global segments for a given vm file.
    // Segment [constant] is virtual which has been already implemented.
 }
  
  std::string getNextSymbolName(){
    std::ostringstream symbol("symbol");
    symbol <<"symbol" <<symcount++;
    return symbol.str();
  }

  
  void setFileName(std::string str) {
    vmfilename = str;
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
    // for segments [local, argument, this, that]:
    //Strategy: get_base_address_of_segment, add_index_to_it, and use it as RAM address, read content of this RAM address
    //into register D and pushDtostack();
    //Algorithm: base_address of segments are in [LCL, ARG, THIS, THAT] registers. To add index to it, put index into D,
    //read base address into A and A=A+D. now D=M and pushDtoStack();
    if((segment == "local") || (segment == "argument") || (segment == "this") || (segment == "that")){
      //put index in D
      outfile << " @" << index << "\r\n D=A\r\n";
      // put base address of sement in A
      putBaseAddressinA(segment); 
      //A=A+D will ensure that target address got into A. read target content in D and pushDtoStack
      outfile << " A=D+A\r\n D=M\r\n" << pushDtoStack();
     }//endif [local|argument| this| that] 

  }//end of writePush
  void writePop(vm_command_type ct, std::string segment, int index){
    if(segment == "constant"){
      decrementSP(); // there is no need to overwrite top of stack as it will be overwritten by next push command.
    }
    if((segment == "local") || (segment == "argument") || (segment == "this") || (segment == "that")){
      //popping a segment seems a bit trickier than pushing a segment.
      // we need D to calculate target address, but D is also needed to hold the value that will be 
      // be transferred from stack to segment. While reading D from M did not change A, popping D from stack
      // will change A. So we will lose the target address and will need D to calculate it. but if we use D to calculate the
      //target address, we will lose the value that is popped.
      //Strategy: calculate target address, store it temporarily in R13. Then PopDfromStack.
      //Then Load A from R13 and M=D.
      
      //Algorithm: 1- Calculate target address. 2- Store in R13. 3- popDfromStack 4- Load A from R13 4- M=D 
      outfile << " @" << index << "\r\n D=A\r\n";
      putBaseAddressinA(segment); 
      //D=A+D will ensure that target address got into D. Store it to R13
      outfile << " D=D+A\r\n @R13\r\n M=D\r\n"; //Target address calculated and stored in R13.
      //Now. 3. popDfromStack
      outfile << popDfromStack(); // 
      //Now load A from R13 and set M=D
      outfile << " @R13\r\n A=M\r\n M=D\r\n "; //All done.
 
     }//endif [local|argument| this| that] 

  }//end of writePop

  void putBaseAddressinA(std::string segment){
    if(segment == "local")
      outfile << "@LCL\r\n A=M\r\n";
    if(segment == "argument")
      outfile << "@ARG\r\n A=M\r\n";
    if(segment == "this")
      outfile << "@THIS\r\n A=M\r\n";
    if(segment == "that")
      outfile << "@THAT\r\n A=M\r\n";
    
  }// end of putBaseAddressinA
 

  ~codewriter(){
    // Write an indefinite loop at the end.
    outfile << "(END)\r\n @END\r\n 0;JMP\r\n";

    outfile.close(); //release resources
  }  


}; // End of class codewriter

#endif
