//implements the parser module for virtual machine implmentation as suggested in the book.
// I also implemented the CodeWriter module in the same file.


#include <iostream>
#include <string.h>
#include <fstream>
#include <sstream>
#include "vm_cmd_type.h" //defines enumerated types for command types of virtual machine
#include <vector>
#include <algorithm>
//using namespace std;

class vmparser{

private:
  std::fstream infile;
  std::string current_command;
  vm_command_type command_type;
  int line_number;
  std::vector<std::string> arith_commands;
  std::vector<std::string> mem_segments;
  std::vector<std::string> tokens;

public:
  vmparser(std::string filename){
    
    //std::cout << " VM parser constructor called with file name" << filename << std::endl;
    infile.open(filename.c_str(), std::ios::in); // string.c_str is used to convert std:string to const char *.
    if(!infile)
      std::cerr << "VM PARSER: Error Opening Input file [" << filename.c_str() << "] \n" ;
    line_number = 0;
    const std::string ac[] = {"add","sub","neg","eq","gt", "lt", "and", "or", "not"};
    arith_commands.insert(arith_commands.begin(), ac, ac+9); // populate the vector
    const std::string mem[] ={"argument","local","static","constant","this","that","pointer","temp"};
    mem_segments.insert(mem_segments.begin(), mem, mem+8);
   
  }
  bool hasMoreCommands(){
    return(!infile.eof());
    
  }

  
  void advance(){
    if(!infile.eof()){
      getline(infile, current_command);
      line_number++;
      getTokens(); 
      command_type = calcCommandType();
      //int i=0;
      //std::cout << "Number of Tokens:" << tokens.size()<< "Token List=";
      //for( std::vector<std::string>::iterator tok=tokens.begin(); tok < tokens.end(); tok++){
      //	std::cout<<i<<":"<<*tok<< "  ";
      //	i++;
      //} //end of for loop
    } //end of if
    //std::cout<<std::endl;
  }//end of advance
  
  void getCurrentString( std::string& line){

    line = current_command;
  }

  int getLineNumber(){
    return(line_number);
  }
  vm_command_type commandType(){
    return command_type;
  }
  vm_command_type calcCommandType(){
    if(tokens.size()== 0)
      return(C_EMPTY);
    std::string arg1 = tokens[0];
    if(arg1 == "//")
      return(C_COMMENT);
    if(arg1 == "return")
      return(C_RETURN);
    if(arg1 == "label")
      return(C_LABEL);
    if(arg1 == "goto")
      return(C_GOTO);
    if(arg1 == "if-goto")
      return(C_IF);
    if(arg1 == "function")
      return(C_FUNCTION);
    if(arg1 == "call")
      return(C_CALL);
    if(arg1 == "push")
      return(C_PUSH);
    if(arg1 == "pop")
      return(C_POP);

    if(std::find(arith_commands.begin(), arith_commands.end(), arg1) != arith_commands.end())
      return(C_ARITHMETIC); // search for arg1 in arithmetic commands. If found there, return type as arithmetic.
    
    
    return(C_INVALID);
  }

  
  void getTokens(){
    std::string s = current_command;
    tokens.clear();// clear tokens.
    size_t pos=0, start=0, len=0, maxlen = current_command.length();
    while(pos < maxlen){     
      if((s[pos] == ' ') || (s[pos] == '\r') || (s[pos] == '\n')){
	pos++;
	continue;  //skip white spaces and new lines.
      }
      else{
	start = pos;
	while((pos < maxlen)&&(s[pos] != ' ') && (s[pos] != '\r') && (s[pos] != '\n')){
	  len++;
	  pos++; 
	}//end of inner while counting the non-delimited characters in 
	tokens.push_back(s.substr(start, len)); // put the string in vector of strings.
	len=0; 
	
      }
    }//end of outer while    
  }//end of getTokens();
  
  std::string arg1(){
    if(tokens.size() < 1){
      std::cerr << "PARSE ERROR: LINE NO: " << line_number << " Command name was accessed but does not exist.\n"  ;
      return("");
    }
    return(tokens[0]);
  }
  std::string arg2(){
    if(tokens.size()<2){
      std::cerr << "PARSE ERROR: LINE NO: " << line_number << " Argument 1 of Comamnd [" << arg1() << "] was requested but it does not exist.\n"  ;      
      return("");
    }
    return(tokens[1]);
  }
  std::string arg3(){
    if(tokens.size() < 3){
      std::cerr << "PARSE ERROR: LINE NO: " << line_number << " Argument 2 of Comamnd [" << arg1() << "] was requested but it does not exist.\n"  ;            
      return("");
    }
    return(tokens[2]);
  }
  int getPushPopIndex(){
    if((command_type != C_PUSH) && (command_type != C_POP)){
      std::cerr << "PARSE ERROR: LINE NO: " << " Index for push/pop was requested but this is not a push/pop command.\n"  ;      
      return(-1);
    }
    if(arg3() == "")
      return(-1);
    std::istringstream str(arg3());
    int out;
    str >> out;
    return(out);
  }
  ~vmparser(){
    infile.close();
  }



}; //end of class vmparser


class codewriter{
private:
  std::fstream outfile;
  std::string newfilename;
  
  int symcount;
  //std::string symstr="symbol";
  //std::istringstream symbol(symstr);

  //std::vector<std::string> arith_commands;
  //std::vector<std::string> mem_segments;
  //std::vector<std::string> tokens;
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


int main(int argc, char **argv){
  
  
  //std::cout << "Hello World \n";
  std::string in = "test.vm";
  std::string out ="testout.asm";
  std::string  line;
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
    //std::cout<< "LINE=" << parser.getLineNumber() << ":" << line <<std::endl;
    command_type = parser.commandType();
    //std::cout << "COMMAND TYPE =" << enumlist[command_type] << "ARG1=" << parser.arg1() << "    ARG2=" << parser.arg2()<< std::endl;
    
    switch(command_type){
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
      //std::cout << "ERROR: This command is not in the VM specification: LINE NO: " << parser.getLineNumber()<< std::endl;
      break;
    default:
      //std::cout<<"This command is not implemented." << std::endl;
      break;
    }
  }
}
