#include <vector>
#include <fstream>

#include "common.h"

#include "program.h"
#include "compiler.h"
#include "vm.h"

/*
	Literal: 1234 1234h 010010b
	Register: A B C D
	Address: [123] [ffh] [10100b] [A] [label]
	Label: thing:
	Comment: # blah 	; blah
*/

int main(){
	try{
		std::ifstream file("test.asm");
		file.seekg(0, file.end);
		string input(file.tellg(), '\0');
		file.seekg(0, file.beg);
		file.read(&input[0], input.size());

		auto program = Compile(input);
		
		VM().Run(program);

	}catch(const std::string& e){
		std::cout << "error:" << e << std::endl;
		// return 1;
	}

	return 0;
}