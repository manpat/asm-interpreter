#ifndef INSTRUCTIONS_INL
#define INSTRUCTIONS_INL

#include "program.h"
#include "vm.h"

void CompilerAssert(bool thing, string e){
	if(!thing){
		throw e;
	}
}

void NotImplemented(string thing){
	throw thing + " not implemented";
}

constexpr unsigned int str2int(const char* str, int h = 0){
    return !str[h] ? 5381 : (str2int(str, h+1)*33) ^ str[h];
}
unsigned int str2int(string str){
    return str2int(str.data());
}
constexpr unsigned int operator""_h(const char* str, size_t len){
	return str2int(str);
}

WordType& InstructionTarget::Eval(VM& vm){
	if(deref){
		switch(type){
		case Type::Register: return vm.GetMemory(vm.GetRegister(reg));
		case Type::Value: return vm.GetMemory(value);
		case Type::Location: return vm.GetMemory(value);
		}
	}else{
		switch(type){
		case Type::Register: return vm.GetRegister(reg);
		case Type::Value: return value;
		case Type::Location: return value;;
		}
	}

	CompilerAssert(false, "Fuck");
	throw "";
}

namespace {
	const std::map<string, unsigned long>* labels = nullptr;
}

long long TokenToLiteral(const Token& tok){
	CompilerAssert(tok.type & Token::Literal, "Tried to convert token to literal ("+tok.text+")");
	int base = 10;

	if(tok.type & Token::Hex){
		base = 16;
	}else if(tok.type & Token::Binary){
		base = 2;
	}

	return std::stoll(tok.text, nullptr, base);
}

Register TokenToRegister(const Token& tok){
	switch(str2int(tok.text)){
		case "A"_h: return Register::A;
		case "B"_h: return Register::B;
		case "C"_h: return Register::C;
		case "D"_h: return Register::D;
		case "SP"_h: return Register::SP;
		case "IP"_h: return Register::IP;
		case "FLAGS"_h: return Register::FLAGS;
	}
	CompilerAssert(false, "Unknown register ("+tok.text+")");
	return (Register)0;
}

InstructionTarget&& TokenToDestination(const Token& tok){
	InstructionTarget trg;

	trg.deref = (tok.type & Token::Dereference);

	if(tok.type & Token::Register){
		trg.type = InstructionTarget::Type::Register;
		trg.reg = TokenToRegister(tok);

	}else if(tok.type & Token::Label){
		auto it = labels->find(tok.text);
		CompilerAssert(it != labels->end(), "Undefined label ("+tok.text+")");
		NotImplemented("Label dst target");

	}else if(tok.type & Token::Literal){
		CompilerAssert(trg.deref, "Literals cannot be destinations ("+tok.text+")");
		trg.type = InstructionTarget::Type::Value;
		trg.value = TokenToLiteral(tok);
	}

	return std::move(trg);
}

InstructionTarget&& TokenToSource(const Token& tok){
	InstructionTarget trg;

	trg.deref = (tok.type & Token::Dereference);

	if(tok.type & Token::Register){
		trg.type = InstructionTarget::Type::Register;
		trg.reg = TokenToRegister(tok);

	}else if(tok.type & Token::Label){
		auto it = labels->find(tok.text);
		CompilerAssert(it != labels->end(), "Undefined label ("+tok.text+")");
		trg.type = InstructionTarget::Type::Location;
		trg.value = it->second;

	}else if(tok.type & Token::Literal){
		trg.type = InstructionTarget::Type::Value;
		trg.value = TokenToLiteral(tok);
	}

	return std::move(trg);
}

#define DEFOP(name) struct name##Inst : Instruction
#define DEFOPCONS(name) name##Inst(const std::vector<Token>& args)

#define WBINOP(name) DEFOPCONS(name) { \
	CompilerAssert(args.size() == 2, #name " takes two arguments"); \
	dst = TokenToDestination(args[0]); \
	src = TokenToSource(args[1]); \
} InstructionTarget dst, src;

#define RBINOP(name) DEFOPCONS(name) { \
	CompilerAssert(args.size() == 2, #name " takes two arguments"); \
	dst = TokenToSource(args[0]); \
	src = TokenToSource(args[1]); \
} InstructionTarget dst, src;

#define RUNARYOP(name) DEFOPCONS(name) { \
	CompilerAssert(args.size() == 1, #name " takes one argument"); \
	src = TokenToSource(args[0]); \
} InstructionTarget src;

#define WUNARYOP(name) DEFOPCONS(name) { \
	CompilerAssert(args.size() == 1, #name " takes one argument"); \
	dst = TokenToDestination(args[0]); \
} InstructionTarget dst;

DEFOP(Nop) {
	void Go(VM& vm) override {}
};

DEFOP(Debug) {
	void Go(VM& vm) override { vm.PrintState(); }
};
DEFOP(Print) {
	RUNARYOP(Print)
	void Go(VM& vm) override { cout << src.Eval(vm) << endl; }
};

DEFOP(Mov) {
	WBINOP(Mov)
	void Go(VM& vm) override { dst.Eval(vm) = src.Eval(vm); }
};

DEFOP(Jmp) {
	RUNARYOP(Jmp)
	void Go(VM& vm) override {
		// -1 because IP gets incremented regardless
		//	so point at instruction before the one you want 
		//	executed
		vm.GetRegister(Register::IP) = src.Eval(vm)-1;
	}
};

DEFOP(Push) {
	RUNARYOP(Push)
	void Go(VM& vm) override { vm.Push(src.Eval(vm)); }
};
DEFOP(Pop) {
	WUNARYOP(Pop)
	void Go(VM& vm) override { dst.Eval(vm) = vm.Pop(); }
};

DEFOP(Add) {
	WBINOP(Add)
	void Go(VM& vm) override { dst.Eval(vm) += src.Eval(vm); }
};
DEFOP(Neg) {
	WUNARYOP(Neg)
	void Go(VM& vm) override { auto& e = dst.Eval(vm); e = -e; }
};
DEFOP(Sub) {
	WBINOP(Sub)
	void Go(VM& vm) override { dst.Eval(vm) -= src.Eval(vm); }
};
DEFOP(Div) {
	WBINOP(Div)
	void Go(VM& vm) override { dst.Eval(vm) /= src.Eval(vm); }
};
DEFOP(Mul) {
	WBINOP(Mul)
	void Go(VM& vm) override { dst.Eval(vm) *= src.Eval(vm); }
};

DEFOP(And) {
	WBINOP(And)
	void Go(VM& vm) override { dst.Eval(vm) &= src.Eval(vm); }
};
DEFOP(Or) {
	WBINOP(Or)
	void Go(VM& vm) override { dst.Eval(vm) |= src.Eval(vm); }
};
DEFOP(Xor) {
	WBINOP(Xor)
	void Go(VM& vm) override { dst.Eval(vm) ^= src.Eval(vm); }
};
DEFOP(Not) {
	WUNARYOP(Not)
	void Go(VM& vm) override { auto& e = dst.Eval(vm); e = ~e; }
};

DEFOP(Shl) {
	WBINOP(Shl)
	void Go(VM& vm) override { dst.Eval(vm) <<= src.Eval(vm); }
};
DEFOP(Shr) {
	WBINOP(Shr)
	void Go(VM& vm) override { dst.Eval(vm) >>= src.Eval(vm); }
};

DEFOP(Cmp) {
	RBINOP(Cmp)
	void Go(VM& vm) override { 
		auto diff = dst.Eval(vm) - src.Eval(vm);
		vm.flags = 0;

		if(diff == 0){
			vm.flags |= Flags::ZeroFlag;
		}else if(diff < 0){
			vm.flags |= Flags::SignFlag;
		}
	}
};

Instruction* CreateInstruction(const IntermediateStage& istage, const string& inst, const std::vector<Token>& args){
	labels = &istage.labels;

	switch(str2int(inst)){
		case "nop"_h: return new NopInst();
		case "debug"_h: return new DebugInst();
		case "print"_h: return new PrintInst(args);
		case "mov"_h: return new MovInst(args);
		case "jmp"_h: return new JmpInst(args);

		case "push"_h: return new PushInst(args);
		case "pop"_h: return new PopInst(args);

		case "add"_h: return new AddInst(args);
		case "sub"_h: return new SubInst(args);
		case "div"_h: return new DivInst(args);
		case "mul"_h: return new MulInst(args);
		case "neg"_h: return new NegInst(args);

		case "and"_h: return new AndInst(args);
		case "or"_h:  return new OrInst(args);
		case "xor"_h: return new XorInst(args);
		case "not"_h: return new NotInst(args);

		case "shl"_h: return new ShlInst(args);
		case "shr"_h: return new ShrInst(args);

		case "cmp"_h: return new CmpInst(args);

		default: throw "Unknown instruction ("+inst+")";
	}
}

#endif