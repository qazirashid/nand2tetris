 gcc -Wall -o ./assembler ./assembler.c 
 g++ -Wall -o ./parser ./parser.cpp  

 for x in `ls ./testvms/*.vm`
 do 
     echo "Translating $x to ${x%.*}.asm"; 
     ./parser $x ${x%.*}.asm;
     echo "Assembling ${x%.*}.asm to ${x%.*}.hack"
     ./assembler ${x%.*}.asm ${x%.*}.hack

 done
