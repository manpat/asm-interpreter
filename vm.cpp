#include "vm.h"
#include "program.h"

void assert(bool cond, string thing){
	if(!cond) throw "VM: " + thing;
}

VM::VM() 
	: stackPtr(GetRegister(Register::SP))
	, instructionPtr(GetRegister(Register::IP))
	, flags(GetRegister(Register::FLAGS))
	, instructionCount(0) {

	stackPtr = PoolSize-1;

	for(auto& m: memory){
		m = 0;
	}
}

void VM::Push(WordType v){
	assert(stackPtr > 0, "stack overflow");
	memory[stackPtr--] = v;
}

WordType VM::Pop(){
	assert(stackPtr < PoolSize-1, "stack underflow");
	return memory[++stackPtr];
}

// Get's offset from top of stack
//	1 is the last thing pushed
WordType& VM::GetStack(WordType o){
	assert(stackPtr+o < PoolSize, "access violation");
	return memory[stackPtr+o];
}

WordType& VM::GetMemory(WordType o){
	assert(o < PoolSize, "access violation");
	assert(o >= 0, "access violation");
	return memory[o];
}

WordType& VM::GetRegister(Register r){
	return registers[(uint)r];
}

#include <thread>
#include <chrono>

void VM::Run(Program* program){
	assert(program, "Null program");
	cout << "Running program" << endl;

	instructionPtr = 0;
	while(instructionPtr < (WordType)program->instructions.size()){
		assert(instructionCount < ExecutionCap, "Program hit execution cap");

		auto i = program->instructions[instructionPtr];

		i->Go(*this);
		++instructionPtr;
		++instructionCount;

		using namespace std::literals::chrono_literals;
		std::this_thread::sleep_for(50ms);

		cerr << ".";
	}
	cerr << endl;
}

void VM::PrintState(){
	cout << "[State dump]\n";
	cout << "Registers:\n\t";
	for(auto r: registers){
		cout << r << "  ";
	}

	cout << "\n\nMemory:\n";
	for(auto m: memory){
		cout << m << "  ";
	}
	cout << endl;
}