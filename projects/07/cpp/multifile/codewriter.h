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
#include <stack>

class codewriter{
private:
  std::fstream outfile;
  std::string vmfilename;
  int symcount;
  int stackbase, heapbase, pointerbase, tempbase;
  std::stack<std::string> funcstack;
 
public:  
  codewriter(std::string filename){
    
    //std::cout << "Code Writer constructor called with file name" << filename << std::endl;
    outfile.open(filename.c_str(), std::ios::out);
    if(!outfile)
      std::cerr << "CODE WRITER: Error Opening Output file [" << filename.c_str() << "] \n";
    symcount =0;stackbase=256; heapbase=2048; pointerbase=3; tempbase=5;
    writeinit();
  }
  
  
  void writeinit(){//sets up the memory segments in registers at the begining.
    
    outfile << " @"<<stackbase<<"\r\n D=A\r\n @SP\r\n M=D\r\n"; 
    writeCall("Sys.init", 0);
    // put stackbase=256 in SP. Stack starts at RAM address 256.
    // The ARG, LCL, THIS, THAT segments dependant on the function that is being called.
    // Here we initilize those assuming that we are in main() function.
    //outfile <<" @"<< heapbase << "\r\n D=A\r\n @ARG\r\n M=D\r\n";
    // Segment argument for arguments is initially set to the heap base. We will set aside 256 RAM
    //addresses for argument segment. Base addres of argument segment is saved in the ARG register.
    //outfile <<" @"<< (heapbase + 256) << "\r\n D=A\r\n @LCL\r\n M=D\r\n";
    //Segment local will be mapped to heapbase+256, where argument segment ends. It will be assigned
    //1024 memory addresses. Its base address will be stored in register LCL.
    //outfile <<" @"<< (heapbase + 256 + 1024) << "\r\n D=A\r\n @THIS\r\n M=D\r\n";
    // Segment this starts where segment local ends. It will be assigned 1024
    // memory addresses and its address will be stored in THIS register. 
    //outfile <<" @"<< (heapbase + 256 + 1024 + 1024) << "\r\n D=A\r\n @THAT\r\n M=D\r\n";
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


  void setVMFileName(std::string str) {
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
      return;
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
      return;
    }//endif [local|argument| this| that] 
    if((segment == "temp") || (segment == "pointer")){
	//the sements [temp , pointer] are directly mapped on the RAM in fixed locations.
	// when we get a [ push temp 3], we get the contents of RAM[tempbase + 3] in register D
	// and push D to Stack.
	//Since we know the base address (does not change dynamically) and index in advance we can perform the 
	// the arithmetic here instead of using the hack CPU at run time to do the arithmetic.
      outfile << " @";
	if(segment == "temp")
	  outfile << (tempbase + index);
	else 
	  outfile << (pointerbase + index);
	outfile << "\r\n D=M\r\n"; // contents of target RAM read into register D. now pushDtostack.
	outfile << pushDtoStack(); //done
	return;
    }// edinf push [temp, pointer]
    if(segment == "static"){
      //For static segment, we make use the conention of Assembler to allocate the next available RAM location after R15
      // to variables. So we will declare a new variable and let assembler assign it a location.
      // We need to build a name for the variable. Hack convention is "vmfilename.index".
      //We have the vmfilename set already. So build a name now.
      //std::stringstream new_name(); // new string stream.
      outfile << " @" << buildStaticName(index)<<"\r\n D=M\r\n"<< pushDtoStack(); //read variable into D and pushDtostack     
      return;
    }// endif push [static]
    
    std::cerr << "WARNING: Code Writer encountered unknown segment name [" << segment << "] in push command\n";

       
  }//end of writePush
  void writePop(vm_command_type ct, std::string segment, int index){
    if(segment == "constant"){
      decrementSP(); // there is no need to overwrite top of stack as it will be overwritten by next push command.
      return;
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
      return;
     }//endif pop[local|argument| this| that] 
    if((segment == "temp") || (segment == "pointer")){
        // first popDfromstack, put the target address in A, and then M=D
	outfile << popDfromStack() << " @"; // Now get the address.
	if(segment == "temp")
	  outfile << (tempbase + index);
	else 
	  outfile << (pointerbase + index);
	outfile << "\r\n M=D\r\n"; // Target RAM is set to D, which was the top of the Stack.
	return;
    }// edinf pop[temp, pointer]
    if(segment == "static"){
      outfile << popDfromStack();
      outfile << " @" << buildStaticName(index)<<"\r\n M=D\r\n";
      return;
    }// endif pop[static]
    
    std::cerr << "WARNING: Code Writer encountered unknown segment name [" << segment << "] in pop command\n";


  }//end of writePop

  void putBaseAddressinA(std::string segment){
    if(segment == "local")
      outfile << "@LCL";
    if(segment == "argument")
      outfile << "@ARG";
    if(segment == "this")
      outfile << "@THIS";
    if(segment == "that")
      outfile << "@THAT";
    outfile << "\r\n A=M\r\n";
  }// end of putBaseAddressinA
 
  std::string buildStaticName(int index){
    std::ostringstream name(vmfilename);
    name << vmfilename << "." << index;
    return(name.str());
  }

  void writeBranch(vm_command_type ct, std::string command,  std::string loc){
    std::string label;
    label = getLabelPrefix() + "$" + loc;

    if(ct == C_LABEL)
      outfile << "(" << label <<")\r\n";
    if(ct == C_GOTO)
      outfile << "@" << label << "\r\n 0;JMP \r\n";
    if(ct == C_IF){
      //std::string symbol1 = getNextSymbolName(); //get two symbols to manage branching.
      //std::string symbol2 = getNextSymbolName();
      outfile << popDfromStack(); //put top of stack to D
      //outfile << incrementSP(); //pop the top most element
      //is necessary because we just want to look at the top element, not pop the top element.
      outfile << "@" << label << "\r\n D;JNE\r\n" ;

    }
  
  }
  void writeFuncDef(std::string f, int k){ //write a definition of function with n_locals.
     
    funcstack.push(f); // pushing function names for constructing label names later.
    int i;
    //write label for function.
    outfile << "(" << f << ")\r\n D=0\r\n"; //Here starts the function definition.
    // D is set to 0 so that local variables can be initialized at stack.
    for(i = 0; i< k; i++){
      outfile << "// pushing local varialbe " << i << "\r\n"; 
      outfile<< pushDtoStack(); 
    } //pushed k loacal variables to stack each initialized as zero.
  }
  void writeCall(std::string f, int n){
    std::string return_label;
    return_label = getLabelPrefix() + "$" + getNextSymbolName();
    outfile << "// preparing new call to function. \n // pushing return address to stack \r\n";
    outfile << " @" <<return_label << "\r\n D=A\r\n" << pushDtoStack(); //pushing the return address to stack.
    outfile << "// push LCL \r\n @LCL\r\n D=M\r\n" << pushDtoStack();  //push LCL(current_value) 
    //symbol LCL is a constant. This constant is not pushed. The value in register LCL is pushed.
    outfile << "// pushing ARG \r\n @ARG\r\n D=M\r\n"  << pushDtoStack(); //push ARG
    outfile << "// pushing THIS\r\n  @THIS\r\n D=M\r\n" << pushDtoStack(); //push THIS
    outfile << "// pushing THAT\r\n @THAT\r\n D=M\r\n" << pushDtoStack(); //push That 
    outfile << "//LCL=SP \r\n  @SP\r\n D=M\r\n @LCL\r\n M=D\r\n";   //read current contents of SP and set LCL to these contents.
    //n5 = n + 5; // preparing to get base address of ARG. // this arithmetic can be done here. VM does not need to do it.
    // To calculate ARG we need to subtract n+5 from SPs currents contents.
    // push current contents of SP to stack. put n+5 in D and push D to stack. write a sub command.
    // pop D from top of stack and store it in ARG.
    outfile << "// calculating ARG base address\r\n @SP\r\n D=M\r\n" << pushDtoStack(); //current contents of SP on top of stack now.
    outfile << " @" << (n + 5) << "\r\n D=A\r\n" << pushDtoStack(); // n+5 on top of stack now.
    writeArithmetic("sub"); // wrote sub command. Now Top of Stack contains SP - n -5
    outfile << popDfromStack(); // SP - n-5 is in D
    outfile << "//saving ARG base address\r\n @ARG\r\n M=D\r\n"; //ARG = SP-n-5
    outfile << " //Jumping to function entry point\r\n @" << f << "\r\n 0;JMP\r\n";
    outfile << "// Putting return Label\r\n"<<"(" << return_label << ")\r\n";


  }
  void writeReturn(){
    //if(funcstack.empty()){
    //std::cerr << "A return from what? \n";
    //return;
    //}
    //funcstack.pop(); // when returning from a function, pop its names so that labels are constructed properly after this function.
    outfile << "//preparing to return from function \r\n";
    outfile << "//FRAME = LCL (FRAME=R13) \r\n";
    outfile << " @LCL\r\n D=M\r\n @R13\r\n M=D\r\n"; //stored contents of LCL into R13.
    outfile <<"// putting the returned result on top of stack of calling function.\r\n"; //popping result from called stack and pushing it to calling stack.
    outfile << popDfromStack() << " @ARG\r\n A=M\r\n M=D\r\n"; // We went to location where ARG's contents were pointing and stored D there. 
    //This location was the base address of ARG segment of called function and hence was the next location to the top of calling function stack.
    outfile << " // restoring Stack pointer \r\n";
    outfile << " @ARG\r\n D=M \r\n D=D+1\r\n @SP\r\n M=D\r\n";
    // The ARG was pointing at the top of stack of calling function. We got that address into D.
    // but since the called function has put a value of top of old stack we incremented D and set SP to D.
    //SP is now restored and points to top of calling function stack with top value being the one returned by the called function.

    outfile << "// restoring THAT\r\n"; // FRAME is in R13. so R13 currently contains (address+1) where THATs contents were stored previously.
    outfile << " @R13\r\n M=M-1\r\n A=M \r\n D=M \r\n @THAT\r\n M=D\r\n"; 
    // We decrement R13 so that it pointed to THATs stored contents. Then we loaded A with that address and braught contents of memory to D.
    //Then we loaded A with THAT and put D there. effectively restoring THATs contents before call.
    outfile << "// restoring THIS\r\n";    
    outfile << " @R13\r\n M=M-1\r\n A=M \r\n D=M \r\n @THIS\r\n M=D\r\n";
    //Did the same trick as before. This time restoring old contents of THIS
    outfile << "// restoring ARG\r\n";
    outfile << " @R13\r\n M=M-1\r\n A=M \r\n D=M \r\n @ARG\r\n M=D\r\n"; 
    //retored ARG
    outfile << "// restoring LCL\r\n";
    outfile << " @R13\r\n M=M-1\r\n A=M \r\n D=M \r\n @LCL\r\n M=D\r\n"; // retored LCL
    // if we decrement R13 just once more its contents will point at the value of returned address. 
    outfile << "// Getting Return aAddress \r\n";
    outfile << " @R13\r\n M=M-1\r\n A=M \r\n D=M\r\n"; //return address is in D now.
    //everything seems ready. Lets just get return address into A and Jump.
    outfile << " // All ready. Eyes closed. jumping now.\r\n";
    outfile <<  " A=D \r\n 0;JMP\r\n"; //hopefully  we will get there where we wanted to go.

    
  }
  std::string getLabelPrefix(){
    if(funcstack.empty())
      return("null");
    else
      return(funcstack.top());
  }
  ~codewriter(){
    // Write an indefinite loop at the end.
    //outfile << "(END)\r\n @END\r\n 0;JMP\r\n";

    outfile.close(); //release resources
  }

}; // End of class codewriter

#endif
