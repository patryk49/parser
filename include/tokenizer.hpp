#pragma once

#include <stdio.h>
#include <stdlib.h>

#include "SPL/Arrays.hpp"
#include "SPL/Allocators.hpp"


constexpr sp::Range<const char> KeywordName[] = {
// COMPILER DIRECTIVES
	sp::range("main"), sp::range("import"),
	sp::range("if"), sp::range("while"), sp::range("for"),
	sp::range("at"),
	sp::range("assert"),
	sp::range("run"), sp::range("inline"),
	sp::range("size"), sp::range("len"),
	sp::range("set"), sp::range("unset"),
	sp::range("asm"), sp::range("insert"),

// KEYWORDS
	sp::range("proc"), sp::range("class"),
	sp::range("struct"), sp::range("union"), sp::range("enum"),

	sp::range("if"), sp::range("while"), sp::range("for"),
	sp::range("defer"), sp::range("goto"), sp::range("return"),
	sp::range("else"), sp::range("do"), sp::range("break"), sp::range("continue"),
	
	sp::range("bytesof"), sp::range("alignof"), sp::range("exists"),
};

enum class NodeType : uint16_t{
// COMPILER DIRECTIVES ----------------------
	Main, Import,
	StaticIf, StaticWhile, StaticFor,
	At,
	StaticAssert,
	StaticRun, Inline,
	StaticSize, StaticLen,
	Set, Unset,
	Asm, Insert,

// KEYWORDS -------------------------------
	Proc, Class,
	Struct, Union, Enum,

	If, While, For,
	Defer, Goto, Return,
	Else, Do, Break, Continue,

	// OPERATORS
	Bytesof, Alignof, Exists,
// END OF KEYWORDS ------------------------
	// UNARY PREFIX OPERATORS
	Plus, Minus,
	GetAddress, Dereference,
	Range, ViewRange,
	LogicNot, BitNot,
	
	// EXPRESSION OPENING SYMBOLS
	Comma, Slice,
	OpenPar, OpenBrace, OpenBracket,
	GetProcedure, GetSomethingInBraces, GetField,

	// BINARY OPERATORS
	LogicOr,
	LogicAnd,
	Equal, NotEqual, Lesser, Greater, LesserEqual, GreaterEqual,
	Add, Subtract,
	Multiply, Divide,
	Modulo,
	Concatenate,
	BitOr, BitNor,
	BitAnd, BitNand,
	BitXor,
	LeftShift, RightShift,

	ArrayAdd, ArraySubtract,
	ArrayMultiply, ArrayDivide,
	ArrayModulo,
	ArrayConcatenate,
	ArrayBitOr, ArrayBitNor,
	ArrayBitAnd, ArrayBitNand,
	ArrayBitXor,
	ArrayLeftShift, ArrayRightShift,

	CarryAdd, BorrowSubtract,
	WideMultiply, ModuloDivide,
	Access,

	// ASSIGNS
	Assign, ExpandAssign,
	
	AddAssign, SubtractAssign,
	MultiplyAssign, DivideAssign,
	ModuloAssign,
	ConcatenateAssign,
	BitOrAssign, BitNorAssign,
	BitAndAssign, BitNandAssign,
	BitXorAssign,
	LeftShiftAssign, RightShiftAssign,

	ArrayAddAssign, ArraySubtractAssign,
	ArrayMultiplyAssign, ArrayDivideAssign,
	ArrayModuloAssign,
	ArrayConcatenateAssign,
	ArrayBitOrAssign, ArrayBitNorAssign,
	ArrayBitAndAssign, ArrayBitNandAssign,
	ArrayBitXorAssign,
	ArrayLeftShiftAssign, ArrayRightShiftAssign,

	// OTHER RIGHT TO LEFT OPERATIONS
	Ternary, Cast, Reinterpret,
	
	// STATEMENTS
	OpenScope, ArrayLiteral, FixedArray, FiniteArray,
	ClosePar, CloseBrace, CloseBracket, CloseDoubleBracket,

	GotoAddress,
	Switch, StaticSwitch,

	DoubleColon, TripleColon, DoubleColonAmpersand,
	Variable, Constant,
	ExpandedVariable, ExpandedConstant,

	GotoInstruction, BreakIterator, ContinueIterator,

	// OTHER
	Attribute, Label, Deduction,
	PredefScope,

	// LITERALS
	Integer, Unsigned, Float, Double, Character, String, EmptyArray,

	// EXPRESSIONS
	Name, Call, Subscript,
	Conversion, DirectCast, WhileOfDo,
	Literal,

	// CLASS NODES
	Pointer, ViewPointer,


	// TOKEN ONLY
	Null, Terminator, Colon,

	// SPECIFICATIONS
	OutputParameter, DereferenceOutput,
	AutoParameterPack,
};






// TOKEN CLASS
union Node{
// DATA TYPES
	union Data{
		size_t index;
		size_t size;
		uint64_t u64;
		uint32_t u32_array[2];
		uint16_t u16_array[4];
		uint8_t u8_array[8];
		float f32;
		double f64;
	};
	
	Node() noexcept{}
	Node(const Node &rhs) noexcept = default;
	Node(NodeType t, uint32_t p) noexcept : type{t}, pos{p} {}


// DATA MEMBERS
	struct{
		NodeType type;
		uint16_t u16;
		uint32_t pos;

		Data data;
	};
	char strpart[16];
};

template<> struct std::is_trivial<Node>{ constexpr static bool value = true; };









SP_CSI bool is_valid_name_char(char c){
	return (c>='a' && c<='z') || (c>='A' && c<='Z') || (c>='0' && c<='9') || c=='_';
}

SP_CSI bool is_valid_first_name_char(char c){
	return (c>='a' && c<='z') || (c>='A' && c<='Z') || c=='_';
}

SP_CSI bool is_number(char c) noexcept{ return (uint8_t)(c-'0') < 10; }

SP_CSI bool is_whitespace(char c) noexcept{ return c==' ' || c=='\n' || c=='\t' || c=='\v'; }

SP_CSI bool is_keyword_statement(NodeType type) noexcept{
	return (
		(uint32_t)type >= (uint32_t)NodeType::Proc && (uint32_t)type <= (uint32_t)NodeType::Exists
	);
}


// TO DO: add utf-8 support
uint32_t get_char_from_iterator(const char **inpIter) noexcept{
	uint32_t c = **inpIter;
	++*inpIter;

	if (c=='\'' || c== '\"' || c=='\n' || c=='\0') return (uint32_t)-1; // -1 means error

	if (c == '\\')
		switch (**inpIter){
		case '\'':
		case '\"': ++*inpIter;
			return *(*inpIter-1);
		case '0' ... '9':
			c = strtol(*inpIter, (char **)inpIter, 10);
			if (c > 127){
				// PLACEHOLDER FOR: translate to utf-8
				return 0;
			}
			return c;
		case 'n': ++*inpIter;
			return '\n';
		case 't': ++*inpIter;
			return '\t';
		case 'v': ++*inpIter;
			return '\v';
		case '\n':
		case '\v': ++*inpIter;
			return (uint32_t)-2; // -2 means that it should be skipped
		}
	return c;
}


// TO DO: save number of the required bits for integer into the u16 field
// TO DO: add separator syntax from C++
Node get_number_token_from_iterator(const char **inpIter, uint32_t pos) noexcept{
	Node node{NodeType::Integer, pos};
	node.u16 = 0;

	const char *prevInpIter = *inpIter;

	char c = **inpIter;
	if (c == '.') goto ParseFloat;

	if (c == '0'){
		++*inpIter;
		switch (**inpIter){
		case 'x':
			node.data.u64 = (uint64_t)strtoul(*inpIter+1, (char **)inpIter, 16);
			goto ReturnInt;
		
		case 'o':
			node.data.u64 = (uint64_t)strtoul(*inpIter+1, (char **)inpIter, 8);
			goto ReturnInt;

		case 'b':
			node.data.u64 = (uint64_t)strtoul(*inpIter+1, (char **)inpIter, 2);
			goto ReturnInt;

		default: break;
		}
	}
	{
		node.data.u64 = (uint64_t)strtoul(*inpIter, (char **)inpIter, 10);
		
		if (**inpIter == '.' && is_number(*(*inpIter+1))){
		ParseFloat:
			node.type = NodeType::Double;
			node.data.f64 = strtod(prevInpIter, (char**)inpIter);
		}
	}

ReturnInt:
	if (**inpIter == 'u'){
		++*inpIter;
		node.type = NodeType::Unsigned;
	} else if (**inpIter == 'f'){
		++*inpIter;
		node.data.f32 = node.type==NodeType::Integer ? (float)node.data.u64 : (float)node.data.f64;
		node.type = NodeType::Float;
	}
	return node;
}




char *code_text;

void print_codeline(const char *text, size_t position) noexcept{
	size_t row = 0;
	size_t col = 0;
	size_t row_position = 0;

	for (size_t i=0; i!=position; ++i){
		++col;
		if (text[i]=='\n' || text[i]=='\v'){
			row_position += col;
			++row;
			col = 0;
		}
	}
	printf(" -> row: %lu, column: %lu\n", row, col);

	putchar('>');
	putchar('\n');
	
	putchar('>');
	putchar(' ');
	putchar(' ');
	for (size_t i=row_position;; ++i){
		char c = text[i];
		if (c=='\0' || c=='\n' || c=='\v') break;
		putchar(c);
	}
	putchar('\n');

	putchar('>');
	putchar(' ');
	putchar(' ');
	for (size_t i=0; i!=col; ++i) putchar(' ');
	putchar('^');
	putchar('\n');
	putchar('\n');
}


void raise_error(const char *msg, uint32_t pos) noexcept{
	fprintf(stderr, "error: \"%s\"", msg);
	print_codeline(code_text, pos);
	exit(1);
}

void raise_error(const char *msg0, const char *msg1, uint32_t pos) noexcept{
	fprintf(stderr, "error: \"%s%s\"", msg0, msg1);
	print_codeline(code_text, pos);
	exit(1);
}
















// array that stores names
sp::DynamicArray<char, sp::MallocAllocator<>> names;


sp::DynamicArray<Node, sp::MallocAllocator<>> make_tokens(const char *input) noexcept{
	sp::DynamicArray<Node, sp::MallocAllocator<>> tokens;
	sp::FiniteArray<uint32_t, 32> scopes;
	Node curr;
	curr.pos = 0;
	bool bracket_expression = false;
	for (;;){
		const char *prevInput = input;
		switch (*input){
		case '=': ++input;
			if (*input == '='){
				curr.type = NodeType::Equal;
				++input;
				goto AddToken;
			}
			if (!is_whitespace(*(input-2))){
				NodeType prev = sp::back(tokens).type;
				if (NodeType::Add<=prev && prev<=NodeType::ArrayRightShift){
					sp::back(tokens).type = (NodeType)(
						(uint16_t)prev + ((uint16_t)NodeType::AddAssign-(uint16_t)NodeType::Add)
					);
					goto Break;
				}
			}
			curr.type = NodeType::Assign;
			goto AddToken;


		case '+': ++input;
			if (*input == '%'){
				curr.type = NodeType::CarryAdd;
				++input;
			} else if (is_number(*input)){
				curr = get_number_token_from_iterator(&input, curr.pos);
			} else{
				curr.type = NodeType::Add;
			}
			goto AddToken;


		case '-': ++input;
			if (*input == '>'){
				curr.type = NodeType::Cast;
				++input;
			} else if (*input == '%'){
				curr.type = NodeType::BorrowSubtract;
				++input;
			} else if (is_number(*input)){
				curr = get_number_token_from_iterator(&input, curr.pos);
				switch (curr.type){
				case NodeType::Double:
					curr.data.f64 = -curr.data.f64;
					break;
				case NodeType::Float:
					curr.data.f32 = -curr.data.f32;
					break;
				default:
					curr.data.u64 = -curr.data.u64;
					break;
				}
			} else{
				curr.type = NodeType::Subtract;
			}
			goto AddToken;


		case '*': ++input;
			if (*input == '%'){
				curr.type = NodeType::WideMultiply;
				++input;
			} else
				curr.type = NodeType::Multiply;
			goto AddToken;


		case '/': ++input;
			if (*input == '/'){
				++input;
				while (*input && *input!='\n') ++input;
				break;
			}
			if (*input == '*'){
				++input;
				size_t depth = 1;
				for (;;){
					if (*input == '\0') break;
					if (input[0]=='*' && input[1]=='/'){
						--depth;
						input += 2;
						if (!depth) break;
						continue;
					}
					if (input[0]=='/' && input[1]=='*'){
						++depth;
						input += 2;
						continue;
					}
					++input;
				}
				break;
			}
			if (*input == '%'){
				curr.type = NodeType::ModuloDivide;
				++input;
			} else if (*input == '>'){
				curr.type =  NodeType::Reinterpret;
				++input;
			} else{
				curr.type = NodeType::Divide;
			}
			goto AddToken;


		case '%': ++input;
			if (*input == '%'){
				curr.type = NodeType::Concatenate;
				++input;
			} else
				curr.type = NodeType::Modulo;
			goto AddToken;


		case '|': ++input;
			if (*input == '|'){
				curr.type = NodeType::LogicOr;
				++input;
			} else
				curr.type = NodeType::BitOr;
			goto AddToken;


		case '&': ++input;
			if (*input == '&'){
				curr.type = NodeType::LogicAnd;
				++input;
			} else
				curr.type = NodeType::BitAnd;
			goto AddToken;


		case '~': ++input;
			if (*input == '|'){
				++input;
				curr.type = NodeType::BitNor;
			} else if (*input == '&'){
				++input;
				curr.type = NodeType::BitNand;
			} else{
				curr.type = NodeType::BitNot;
			}
			goto AddToken;


		case '<':
			++input;
			if (*input == '='){
				curr.type = NodeType::LesserEqual;
				++input;
			} else if (*input == '<'){
				++input;
				if (input[1] == '='){
					curr.type = NodeType::LeftShiftAssign;
					++input;
				} else
					curr.type = NodeType::LeftShift;
			} else{
				curr.type = NodeType::Lesser;
			}
			goto AddToken;


		case '>':
			++input;
			if (*input == '='){
				curr.type = NodeType::GreaterEqual;
				++input;
			} else if (*input == '>'){
				++input;
				if (input[1] == '='){
					curr.type = NodeType::RightShiftAssign;
					++input;
				} else
					curr.type = NodeType::RightShift;
			} else if (*input == '<'){
				++input;
				if (*input == '='){
					++input;
					curr.type = NodeType::BitXorAssign;
				} else{
					curr.type = NodeType::BitXor;
				}
			}	else
				curr.type = NodeType::Greater;
			goto AddToken;


		case '!':
			++input;
			if (*input == '='){
				curr.type = NodeType::NotEqual;
				++input;
			} else
				curr.type = NodeType::LogicNot;
			goto AddToken;


		case '(':
			curr.type = NodeType::OpenPar;
			++input;
			goto AddToken;
		

		case ')':
			curr.type = NodeType::ClosePar;
			++input;
			goto AddToken;
		
		
		case '{':
			sp::push_value(scopes, sp::len(tokens));
			curr.type = NodeType::OpenBrace;
			++input;
			goto AddToken;
		
		
		case '}':
			// [[unlikely]] if (sp::is_empty(scopes)) raise_error("too many closing braces", curr.pos);
			sp::pop(scopes);
			curr.type = NodeType::CloseBrace;
			++input;
			goto AddToken;
		
		
		case '[': ++input;
			if (*input == '['){
				curr.type = NodeType::FiniteArray;
				++input;
			} else if (*input == ']'){
				curr.type = NodeType::Range;
				++input;
			} else{
				bracket_expression = !is_whitespace(*input); 
				input += !bracket_expression;
				
				curr.type = NodeType::OpenBracket;
			}
			goto AddToken;
		
		
		case ']': ++input;
			if (*input == ']'){
				curr.type = NodeType::CloseDoubleBracket;
				++input;
			} else{
				if (
					bracket_expression
					&& !is_whitespace(*(input-2))
					&& tokens[sp::len(tokens)-2].type==NodeType::OpenBracket
				){
					NodeType op = sp::back(tokens).type;
					if (NodeType::Add<=op && op<=NodeType::RightShift){
						sp::pop(tokens);
						sp::back(tokens).type = (NodeType)(
							(uint16_t)op + ((uint16_t)NodeType::ArrayAdd-(uint16_t)NodeType::Add)
						);
						goto Break;
					} else if (op == NodeType::ViewPointer){
						sp::pop(tokens);
						sp::back(tokens).type = NodeType::ViewRange;
						goto Break;
					}
				}
				curr.type = NodeType::CloseBracket;
			}
			goto AddToken;


		case '^':
			curr.type = NodeType::ViewPointer;
			++input;
			goto AddToken;
		
		
		case '?':
			curr.type = NodeType::Ternary;
			++input;
			goto AddToken;
		
		case ':':
			++input;
			if (*input == ':'){
				curr.type = NodeType::DoubleColon;
				++input;
				if (*input == ':'){
					curr.type = NodeType::TripleColon;
					++input;
				} else if (*input == '='){
					curr.type = NodeType::Constant;
					++input;
				} else if (*input == '&'){
					curr.type = NodeType::DoubleColonAmpersand;
					++input;
				}
			} else if (*input == '='){
				curr.type = NodeType::Variable;
				++input;
			} else {
				curr.type = NodeType::Colon;
			}
			goto AddToken;
		

		case ',':
			++input;
			curr.type = NodeType::Comma;
			break;
			goto AddToken;


		case '.':
			++input;
			if (*input == '.'){
				++input;
				curr.type = NodeType::Slice;
			} else if (*input == '('){
				++input;
				curr.type = NodeType::GetProcedure;
			} else if (*input == '['){
				++input;
				curr.type = NodeType::GetField;
			} else if (is_number(*input)){
				--input;
				curr = get_number_token_from_iterator(&input, curr.pos);
			} else{
				curr.type = NodeType::Access;
			}
			goto AddToken;


		case ';':
			if (!sp::is_empty(scopes)) tokens[(size_t)sp::back(scopes)].type = NodeType::OpenScope;
			++input;
			curr.type = NodeType::Terminator;
			goto AddToken;
		
		
		case '\'':{
				++input;
				const char *input_backup = input;
				uint32_t c = get_char_from_iterator(&input);
				if (*input != '\''){
					curr.type = NodeType::Dereference;
					input = input_backup;
					goto AddToken;
				}
	// TO DO: add utf-8 support
				curr.type = NodeType::Character;
				curr.data.u64 = (uint32_t)c;

				++input;
				goto AddToken;
			}
		
	// TO DO: add raw string literals
	// TO DO: add utf-8 support
		case '\"':{
			++input;
			curr.type = NodeType::String;
			curr.data.index = sp::len(names);
			curr.u16 = 0;
			for (; *input!='\"'; ++curr.u16){
				if (*input == '\0') raise_error("unfinished string literal", curr.pos);

				uint32_t c = get_char_from_iterator(&input);
				if (c == (uint32_t)-1) raise_error("invalid character at string literal", curr.pos);
				if (c == (uint32_t)-2) continue;
				push_value(names, (char)c);
			}
			++input;
			goto AddToken;
		}

		case '#':{
			++input;
			const char *start = input;
			for (; is_valid_name_char(*input); ++input);
			sp::Range<const char> text{start, input-start};

			for (uint32_t i=0; i<(uint32_t)NodeType::Proc; ++i)
				if (text == KeywordName[i]){
					curr.type = (NodeType)i;
					goto AddToken;
				}
			raise_error("wrong compile time directive", curr.pos);
		}
		
		case ' ':
		case '\t':
		case '\n':
			++input;
			goto Break;


		default:
			if (is_valid_first_name_char(*input)){
				const char *start = input;
				while (is_valid_name_char(*++input));
				sp::Range<const char> text{start, input-start};
				if (sp::len(text) > UINT16_MAX) raise_error("name is too long", curr.pos);
		
				for (uint32_t i=(uint32_t )NodeType::Proc; i<=(uint32_t)NodeType::Exists; ++i)
					if (text == KeywordName[(size_t)i]){
						curr.type = (NodeType)i;
						goto AddToken;
					}

				curr.type = NodeType::Name;
				curr.u16 = sp::len(text);
				curr.data.index = sp::len(names);
				push_range(names, text);

				goto AddToken;
			}
			if (is_number(*input)){
				curr = get_number_token_from_iterator(&input, curr.pos);
				goto AddToken;
			}
			curr.type = NodeType::Null;
			push_value(tokens, curr);
			return tokens;
		}
	AddToken:
		push_value(tokens, curr);
	Break:
		curr.pos += input - prevInput;
	}
}

