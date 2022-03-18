#include "expr_parser.hpp"





int main(int argc, char **argv){
	sp::DynamicArray<char, sp::MallocAllocator<>> text;
	
	FILE *file = stdin;
	if (argc == 2){
		file = fopen(argv[1], "r");
		if (!file){
			fputs("file not found\n", stderr);
			return 1;
		}
	}

	// read text
	for (;;){
		int c = getc(file);
		if (c == EOF) break;
		sp::push_value(text, (char)c);
	}

	code_text = sp::beg(text);

	auto tokens = make_tokens(sp::beg(text));
	auto nodes = parse_function(sp::beg(tokens));

	for (auto it=sp::beg(nodes); it!=sp::end(nodes); ++it){
		printf("%5u :: ", (uint32_t)it->pos);
		if (it->type < NodeType::Proc){
			printf("compiler directive -> %s", KeywordName[(size_t)it->type].ptr);
		} else if (it->type <= NodeType::Exists){
			printf("keyword -> %s", KeywordName[(size_t)it->type].ptr);
		} else{
			switch (it->type){
			case NodeType::Access:
				printf("access field");
				break;
			case NodeType::Plus:
				printf("unary plus");
				break;
			case NodeType::Minus:
				printf("unary minus");
				break;
			case NodeType::Cast:
				printf("cast types");
				break;
			case NodeType::Reinterpret:
				printf("reinterpret types");
				break;
			case NodeType::Dereference:
				printf("dereference");
				break;
			case NodeType::GetAddress:
				printf("get address");
				break;
			case NodeType::LogicOr:
				printf("logic or");
				break;
			case NodeType::LogicAnd:
				printf("logic and");
				break;
			case NodeType::Range:
				printf("range");
				break;
			case NodeType::ViewRange:
				printf("viewing range");
				break;
			case NodeType::Equal:
				printf("equal");
				break;
			case NodeType::NotEqual:
				printf("not equal");
				break;
			case NodeType::Lesser:
				printf("lesser");
				break;
			case NodeType::Greater:
				printf("greater");
				break;
			case NodeType::LesserEqual:
				printf("lesser of equal");
				break;
			case NodeType::GreaterEqual:
				printf("greater or equal");
				break;
			case NodeType::Add:
				printf("add");
				break;
			case NodeType::Subtract:
				printf("subtract");
				break;
			case NodeType::Divide:
				printf("divide");
				break;
			case NodeType::Multiply:
				printf("multiply");
				break;
			case NodeType::Modulo:
				printf("modulo");
				break;
			case NodeType::Concatenate:
				printf("concatenation");
				break;
			case NodeType::BitOr:
				printf("bitwise or");
				break;
			case NodeType::BitNor:
				printf("bitwise nor");
				break;
			case NodeType::BitAnd:
				printf("bitwise and");
				break;
			case NodeType::BitNand:
				printf("bitwise nand");
				break;
			case NodeType::BitXor:
				printf("bitwise xor");
				break;
			case NodeType::LeftShift:
				printf("bitwise left shift");
				break;
			case NodeType::RightShift:
				printf("bitwise right shift");
				break;
			case NodeType::ArrayAdd:
				printf("array add");
				break;
			case NodeType::ArraySubtract:
				printf("array subtract");
				break;
			case NodeType::ArrayDivide:
				printf("array divide");
				break;
			case NodeType::ArrayMultiply:
				printf("array multiply");
				break;
			case NodeType::ArrayModulo:
				printf("array modulo");
				break;
			case NodeType::ArrayConcatenate:
				printf("array concatenation");
				break;
			case NodeType::ArrayBitOr:
				printf("array bitwise or");
				break;
			case NodeType::ArrayBitNor:
				printf("array bitwise nor");
				break;
			case NodeType::ArrayBitAnd:
				printf("array bitwise and");
				break;
			case NodeType::ArrayBitNand:
				printf("array bitwise nand");
				break;
			case NodeType::ArrayBitXor:
				printf("array bitwise xor");
				break;
			case NodeType::ArrayLeftShift:
				printf("array bitwise left shift");
				break;
			case NodeType::ArrayRightShift:
				printf("array bitwise right shift");
				break;
			case NodeType::CarryAdd:
				printf("add with carry");
				break;
			case NodeType::BorrowSubtract:
				printf("subtract with borrow");
				break;
			case NodeType::WideMultiply:
				printf("wide multiply");
				break;
			case NodeType::ModuloDivide:
				printf("divide with remainder");
				break;
			case NodeType::Assign:
				printf("assign");
				break;
			case NodeType::AddAssign:
				printf("add assign");
				break;
			case NodeType::SubtractAssign:
				printf("subtract assign");
				break;
			case NodeType::DivideAssign:
				printf("divide assign");
				break;
			case NodeType::MultiplyAssign:
				printf("multiply assign");
				break;
			case NodeType::ModuloAssign:
				printf("modulo assign");
				break;
			case NodeType::ConcatenateAssign:
				printf("concatenation assign");
				break;
			case NodeType::BitOrAssign:
				printf("bitwise or assign");
				break;
			case NodeType::BitNorAssign:
				printf("bitwise nor assign");
				break;
			case NodeType::BitAndAssign:
				printf("bitwise and assign");
				break;
			case NodeType::BitNandAssign:
				printf("bitwise nand assign");
				break;
			case NodeType::BitXorAssign:
				printf("bitwise xor assign");
				break;
			case NodeType::LeftShiftAssign:
				printf("bitwise left shift assign");
				break;
			case NodeType::RightShiftAssign:
				printf("bitwise right shift assign");
				break;
			case NodeType::ArrayAddAssign:
				printf("array add assign");
				break;
			case NodeType::ArraySubtractAssign:
				printf("array subtract assign");
				break;
			case NodeType::ArrayDivideAssign:
				printf("array divide assign");
				break;
			case NodeType::ArrayMultiplyAssign:
				printf("array multiply assign");
				break;
			case NodeType::ArrayModuloAssign:
				printf("array modulo assign");
				break;
			case NodeType::ArrayConcatenateAssign:
				printf("array concatenation assign");
				break;
			case NodeType::ArrayBitOrAssign:
				printf("array bitwise or assign");
				break;
			case NodeType::ArrayBitNorAssign:
				printf("array bitwise nor assign");
				break;
			case NodeType::ArrayBitAndAssign:
				printf("array bitwise and assign");
				break;
			case NodeType::ArrayBitNandAssign:
				printf("array bitwise nand assign");
				break;
			case NodeType::ArrayBitXorAssign:
				printf("array bitwise xor assign");
				break;
			case NodeType::ArrayLeftShiftAssign:
				printf("array bitwise left shift assign");
				break;
			case NodeType::ArrayRightShiftAssign:
				printf("array bitwise right shift assign");
				break;
			case NodeType::Label:
				printf("label");
				break;
			case NodeType::Declare:
				printf("declare");
				break;
			case NodeType::DeclareConst:
				printf("declare constant");
				break;
			case NodeType::DeclareWeakConst:
				printf("declare weak constant");
				break;
			case NodeType::Name:
				printf("identifier name -> ");
				for (size_t i=it->data.name_index; i!=it->data.name_index+it->u16; ++i) putchar(names[i]);
				break;
			case NodeType::String:
				printf("string -> \"");
				for (size_t i=it->data.name_index; i!=it->data.name_index+it->u16; ++i) putchar(names[i]);
				putchar('\"');
				break;
			case NodeType::Character:
				printf("character -> %c", (char)it->data.u64);
				break;
			case NodeType::Double:
				printf("double precission float -> %lf", it->data.f64);
				break;
			case NodeType::Float:
				printf("single precission float -> %f", it->data.f32);
				break;
			case NodeType::Unsigned:
				printf("unsigned integer -> %lu", it->data.u64);
				break;
			case NodeType::Integer:
				printf("signed integer -> %li", it->data.u64);
				break;
			case NodeType::Comma:
				printf("comma");
				break;
			case NodeType::Null:
				printf("null");
				break;
			case NodeType::Colon:
				printf("colon");
				break;
			case NodeType::Terminator:
				printf("terminator");
				break;
			case NodeType::OpenPar:
				printf("functon call : %d", (int)it->u16);
				break;
			case NodeType::OpenBrace:
				printf("initialization : %d", (int)it->u16);
				break;
			case NodeType::OpenBracket:
				printf("indexed access : %d", (int)it->u16);
				break;
			default:
				printf("print is not implemented for this token");
				break;
			}
		}
		putchar('\n');
	}


	return 0;
}
