#ifndef VM_H
#define VM_H

#include <array>
#include "common.h"

class Program;

enum class Register : uint {
	A, B, C, D, // General purpose
	SP, // Stack pointer
	IP, // Instruction pointer

	FLAGS, // Flags
	Count
};

enum Flags : WordType {
	ZeroFlag = 1<<0,
	SignFlag = 1<<1,
};

struct VM {
	static constexpr WordType PoolSize = 1<<10;
	static constexpr WordType ExecutionCap = 512;

	std::array<WordType, (uint)Register::Count> registers;
	std::array<WordType, PoolSize> memory;
	WordType& stackPtr;
	WordType& instructionPtr;
	WordType& flags;
	WordType instructionCount;

	VM();

	void Push(WordType);
	WordType Pop();
	WordType& GetStack(WordType);
	WordType& GetMemory(WordType);
	WordType& GetRegister(Register);

	void Run(Program*);
	void PrintState();
};

#endif