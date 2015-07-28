#ifndef COMPILER_H
#define COMPILER_H

#include "common.h"
#include <map>

class Program;
Program* Compile(string input);

namespace compiler {
	struct Token {
		enum Type {
			Invalid		= 0,
			Instruction = 1<<0, // mov, sub
			Literal 	= 1<<1, // 1234, 1234h, 1010b, [123], [123h], [101b]
			NewLabel 	= 1<<2, // label:
			Label 		= 1<<3, // label, [label]
			Register 	= 1<<4, // A, B, [A], [B]

			Dereference = 1<<5, // [1234], [label], [A]

			Hex 		= 1<<6, // 1234h, [1234h]
			Binary 		= 1<<7, // 1010b, [1010b]
		};

		string text;
		uint type;
	};

	struct Line {
		std::vector<Token> tokens;
		size_t lineno;
		size_t instno;

		bool hasLabel;
		bool hasInstruction;
	};

	struct IntermediateStage {
		std::vector<Line> lines;
		std::map<string, size_t> labels;
	};
}

#endif