#ifndef PROGRAM_H
#define PROGRAM_H

#include <vector>
#include <cstring>

struct VM;
enum class Register : uint;

struct Instruction {
	virtual void Go(VM&) = 0;
};

struct InstructionTarget {
	enum class Type {
		Register,
		Value,
		Location,
	};

	union {
		Register reg;
		WordType value;
	};
	Type type;
	bool deref;

	WordType& Eval(VM& vm);
};

struct Program {
	std::vector<Instruction*> instructions;
};

#endif