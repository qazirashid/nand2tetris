#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
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

bool hasExtension(const std::string& s, const std::string& suffix);
int getdir (std::string dir, std::vector<std::string> &files);
int hasExtensionC(char* s, char* suffix);


int getdir (std::string dir, std::vector<std::string> &files)
{
  DIR *dp;
  struct dirent *dirp;
  
  if((dp  = opendir(dir.c_str())) == NULL) {
    std::cout << "VM Translator Error:(" << errno << ") opening directory " << dir << std::endl;
    return errno;
  }

  while ((dirp = readdir(dp)) != NULL) {
    if(hasExtension(std::string(dirp->d_name), ".vm"))
      files.push_back(std::string(dirp->d_name));
  }
  closedir(dp);
  return 1;
}


bool hasExtension(const std::string& s, const std::string& suffix){
   return (s.size() >= suffix.size()) && std::equal(suffix.rbegin(), suffix.rend(), s.rbegin());
}

int hasExtensionC(char* s, char* suffix){
  size_t slen, suffixlen, i, j;
  slen = strlen(s);
  suffixlen= strlen(suffix);
  if(slen < suffixlen)
    return(0);
  for(i=suffixlen-1, j=slen-1; i >= 0; i--, j--){
    if( s[j] != suffix[i])
      return(0);
  }
  return(1);
}

int main(int argc, char **argv){
  
  
  //std::cout << "Hello World \n";
  std::string in = "test.vm";
  std::string out ="Xxx.asm";
  std::string under_process = "test.vm";

  std::string  line;
  int lineno;
  std::string target_dir; 
  std::vector<std::string> filesindir;

  vm_command_type command_type;
  //size_t argv1len=0, argv2len=0;

  if(argc >2){
    out.assign(argv[2]);
  }

  codewriter writer(out);
  // There will be one code writer for all vm files. There will be one parser for each  vm file.
  
  
  if(argc >1){
    //argv1len = strlen(argv[1]);
    in.assign(argv[1]); // get input filename from command line argument.
    // test the input name. If it is a file name with exention [.vm]. push it to the vector [filesindir]
    if(hasExtension(in, ".vm")){
      filesindir.push_back(in);
    }
    else{
      getdir(in, filesindir);
      writer.writeInit(); 
      //if only one file needs translation, do not write initialization code. 
      //Otherwise Sys.init will not be available and a call to it will cause undefined behavior.
    }
  }// endif(argc >1)

  while(!filesindir.empty()){
    
    under_process = filesindir.back();
    std::cout << "Processing New File: " << under_process << std::endl;
    filesindir.pop_back();
    vmparser parser(under_process);
    writer.setVMFileName(under_process); // to inform codewriter to construct labels accordingly.


    while(parser.hasMoreCommands()){
      parser.advance();
      parser.getCurrentString(line);
      lineno = parser.getLineNumber();
      //std::cout<< "LINE=" << parser.getLineNumber() << ":" << line <<std::endl;
      command_type = parser.commandType();
      //std::cout << "COMMAND TYPE =" << enumlist[command_type] << "   ARG1=" << parser.arg1() << "    ARG2=" << parser.arg2()<< std::endl; 
    
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
      case C_LABEL: case C_GOTO: case C_IF:
	writer.writeBranch(command_type, parser.arg1(), parser.arg2()); 
	break;
      case C_FUNCTION:
	writer.writeFuncDef(parser.arg2(), parser.getPushPopIndex());
	break;
      case C_RETURN:
	writer.writeReturn();
	break;
      case C_CALL:
	writer.writeCall(parser.arg2(), parser.getPushPopIndex());
	break;
      case C_INVALID:
	std::cerr << "LINE No. " << lineno << " ERROR: The command [" << line << "] is not in the VM specification " << std::endl;
	break;
      default:
	std::cerr<<"LINE NO. "<< lineno <<" WARNING: The command ["<< line << "] is not implemented." << std::endl;
	break;
      }//end of switch(command_type)
    } //end of while(parser.hasMoreCommands)
  }// end of while(!filesindir.empty())
}//end of main()
