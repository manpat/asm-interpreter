#include "compiler.h"
#include "program.h"
#include "common.h"

#include <regex>

using namespace compiler;
using std::regex;
using std::regex_match;

std::vector<string> Linify(string);
Line ParseLine(string);
IntermediateStage ProcessTokens(std::vector<Line>&);
Program* GenerateInstructions(IntermediateStage&);

void CompilerError(size_t lineno, string e){
	throw std::to_string(lineno+1) + ": " + e;
}

Program* Compile(string input){
	auto program = new Program;
	auto lines = Linify(input);

	std::vector<Line> tokenLines;
	size_t lineno = 0;

	for(auto& ls: lines){
		auto l = ParseLine(ls);

		l.lineno = lineno;
		tokenLines.push_back(l);

		++lineno;
	}

	auto istage = ProcessTokens(tokenLines);
	program = GenerateInstructions(istage);
	return program;
}

std::vector<string> Linify(string input){
	std::vector<string> lines;
	size_t npos = 0;

	while((npos = input.find('\n')) != input.npos){
		lines.push_back(input.substr(0, npos));
		input = input.substr(npos+1);
	}
	lines.push_back(input);

	return lines;
}

Line ParseLine(string input){
	static string exclusion(" ,\t");
	Line line;

	size_t beg = 0;
	size_t end = 0;

	auto commentBeg = input.find_first_of("#;");

	while(true) {
		beg = input.find_first_not_of(exclusion, end);
		if(beg >= commentBeg) break;
		end = input.find_first_of(exclusion, beg);

		if(end == input.npos){
			line.tokens.push_back(Token{input.substr(beg), Token::Type::Invalid});
			break;

		}else{
			line.tokens.push_back(Token{input.substr(beg,end-beg), Token::Type::Invalid});
		}

		end++;
	}

	return line;
}

IntermediateStage ProcessTokens(std::vector<Line>& lines){
	regex dereferenceRegex(R"(\[(.+)\])");
	regex literalRegex(R"(\d+|[[:xdigit:]]+h|[01]+b)");
	regex newlabelRegex(R"(([[:alpha:]_.][\w_.]*):)");
	regex wordRegex(R"([[:alpha:]_.][\w_.]*)");
	regex registerRegex(R"(A|B|C|D|SP|IP|FLAGS)");

	IntermediateStage stage;

	for(auto& line: lines){
		line.hasLabel = false;
		line.hasInstruction = false;

		for(size_t i = 0; i < line.tokens.size(); i++){
			auto& t = line.tokens[i];
			std::smatch match;

			// NewLabel must be before any instructions
			if(regex_match(t.text, match, newlabelRegex)){
				if(line.hasInstruction){
					CompilerError(line.lineno, "Labels must be before instructions ("+t.text+")");
				}

				auto s = t.text = match[1];
				t.type |= Token::NewLabel;
				line.hasLabel = true;

				auto& lmap = stage.labels;
				if(lmap.find(s) != lmap.end()){
					CompilerError(line.lineno, "Redeclaration of label ("+s+")");
				}

				lmap[s] = stage.lines.size();

				continue;
			}

			if(regex_match(t.text, match, dereferenceRegex)){
				if(!line.hasInstruction){
					CompilerError(line.lineno, "Dereferences can only be used as arguments ("+t.text+")");
				}

				t.text = match[1];
				t.type |= Token::Dereference;
			}

			if(regex_match(t.text, match, literalRegex)){
				if(!line.hasInstruction){
					CompilerError(line.lineno, "Literals can only be used as arguments ("+t.text+")");
				}

				auto s = match.str();
				t.type |= Token::Literal;

				if(s.back() == 'h'){
					t.type |= Token::Hex;
					t.text = s.substr(0, s.length()-1);
				}else if(s.back() == 'b'){
					t.type |= Token::Binary;
					t.text = s.substr(0, s.length()-1);
				}

			}else if(regex_match(t.text, match, registerRegex)){
				if(!line.hasInstruction){
					CompilerError(line.lineno, "Registers can only be used as arguments ("+t.text+")");
				}

				t.type |= Token::Register;

			}else if(regex_match(t.text, match, wordRegex)){
				if(i == 0 || (line.hasLabel && i == 1)){
					t.type |= Token::Instruction;
					line.hasInstruction = true;

				}else{
					t.type |= Token::Label;
				}

			}else{
				CompilerError(line.lineno, "Unknown token ("+t.text+")");
			}
		}

		line.instno = stage.lines.size();
		if(line.hasInstruction) {
			stage.lines.push_back(line);
		}
	}

	return stage;
}

#include "instructions.inl"

Program* GenerateInstructions(IntermediateStage& istage){
	auto program = new Program();
	program->instructions = {};

	cout << "\n";
	for(auto& l: istage.lines){
		cout << l.lineno << " [" << l.instno << "]: ";
		for(auto& t: l.tokens){
			if(t.type & Token::NewLabel) continue;

			cout<< "(" << std::hex << t.type << std::dec << ") \"" 
				<< t.text << "\"\t";
		}
		cout << endl;
	}

	string inst;
	std::vector<Token> args;
	for(auto& l: istage.lines){
		args.clear();

		for(auto& t: l.tokens){
			// Eat labels
			if(t.type & Token::NewLabel) continue;
			if(t.type & Token::Instruction){
				inst = t.text;
			}else{
				args.push_back(t);
			}
		}

		try{
			program->instructions.push_back(CreateInstruction(istage, inst, args));
		}catch(const string& e){
			CompilerError(l.lineno, e);
		}
	}

	return program;
}
