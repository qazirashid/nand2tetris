
#include <iostream>
#include <string.h>
#include <fstream>
#include <sstream>
#include "vm_cmd_type.h" //defines enumerated types for command types of virtual machine
//#include "codewriter.h"
#include <vector>
#include <algorithm>

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
    if(arg1 == "")
      return(C_EMPTY);
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
