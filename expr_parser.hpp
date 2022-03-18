#include "tokenizer.hpp"

// there are 3 namespace which elements of it cannot collide
// - space of functions
// - space of classes
// - space of variables



struct ParseContext{


	size_t class_start;
};

uint8_t prec_table[] = {
	0,                // Comma,
	18, 18, 18,       // OpenPar, OpenBrace, OpenBracket,
	
	3,                // LogicOr,
	4,                // LogicAnd,
	5, 5, 5, 5, 5, 5, // Equal, NotEqual, Lesser, Greater, LesserEqual, GreaterEqual,
	6, 6,             // Add, Subtract,
	7, 7,             // Multiply, Divide,
	8,                // Modulo,
	9,                // Concatenate,
	10, 10,           // BitOr, BitNor,
	11, 11,           // BitAnd, BitNand,
	12,               // BitXor,
	13, 13,           // LeftShift, RightShift, 
	
	6, 6,             // ArrayAdd, ArraySubtract,
	7, 7,             // ArrayMultiply, ArrayDivide,
	8,                // ArrayModulo,
	9,                // ArrayConcatenate,
	10, 10,           // ArrayBitOr, ArrayBitNor,
	11, 11,           // ArrayBitAnd, ArrayBitNand,
	12,               // ArrayBitXor,
	13, 13,           // ArrayLeftShift, ArrayRightShift,
	
	6, 6,             // CarryAdd, BorrowSubtract,
	7, 7,             // WideMultiply, ModuloDivide,
	18,               // Access,

	1,                // Assign,
	1, 1,             // AddAssign, SubtractAssign,
	1, 1,             // MultiplyAssign, DivideAssign,
	1,                // ModuloAssign,
	1,                // ConcatenateAssign,
	1, 1,             // BitOrAssign, BitNorAssign,
	1, 1,             // BitAndAssign, BitNandAssign,
	1,                // BitXorAssign,
	1, 1,             // LeftShiftAssign, RightShiftAssign, 
	1, 1,             // ArrayAddAssign, ArraySubtractAssign,
	1, 1,             // ArrayMultiplyAssign, ArrayDivideAssign,
	1,                // ArrayModuloAssign,
	1,                // ArrayConcatenateAssign,
	1, 1,             // ArrayBitOrAssign, ArrayBitNorAssign,
	1, 1,             // ArrayBitAndAssign, ArrayBitNandAssign,
	1,                // ArrayBitXorAssign,
	1, 1,             // ArrayLeftShiftAssign, ArrayRightShiftAssign,	
	
	1,                // Ternary,
	16, 16,           // Cast, Reinterpret,

//	18, 18, 18. 18,   // Call, GetFunction, GetElement, GetMember, 
};

SP_CSI size_t right_to_left(NodeType t) noexcept{ return t >= NodeType::Assign; }

template<class T, class A>
SP_CSI void insert(sp::DynamicArray<T, A> &arr, size_t pos, const T &value) noexcept{
	sp::push(arr);
	for (size_t i=sp::len(arr); --i!=pos;) arr[i] = arr[i-1];
	arr[pos] = value;
}

struct ParamInfo{ uint32_t index; NodeType finisher; };

auto parse_function(const Node *token) noexcept{
	sp::DynamicArray<Node, sp::MallocAllocator<>> nodes;
	sp::FiniteArray<uint32_t [2], 32> precs;
	sp::FiniteArray<ParamInfo, 32> parameters;
	sp::push_value(precs, (uint32_t [2]){0, 0});
	sp::push_value(parameters, ParamInfo{UINT32_MAX, NodeType::Null});
	size_t prec_offset = 0;

	for (;;){
		size_t end_pos = sp::len(nodes);

		Node op_node = *token;
		size_t unary_prec = 14 + prec_offset;
		switch (op_node.type){
			case NodeType::Add:
				op_node.type = NodeType::Plus;
				break;
			case NodeType::Subtract:
				op_node.type = NodeType::Minus;
				break;
			case NodeType::Multiply:
				unary_prec += 3;
				op_node.type = NodeType::GetAddress;
				break;
			case NodeType::Range:
			case NodeType::ViewRange:
				unary_prec += 3;
			case NodeType::LogicNot:
			case NodeType::BitNot:
			case NodeType::Dereference:
				break;
			case NodeType::Name:
			case NodeType::Integer:
			case NodeType::Unsigned:
			case NodeType::Float:
			case NodeType::Double:
			case NodeType::Character:
			case NodeType::String:
				sp::push_value(nodes, *token);
				goto Finish;
			case NodeType::OpenPar:
				sp::push_value(parameters, ParamInfo{UINT32_MAX, NodeType::ClosePar});
				prec_offset += 32;
				++token;
				continue;
			case NodeType::OpenBracket:
				sp::push_value(parameters, ParamInfo{end_pos, NodeType::CloseBracket});
				prec_offset += 32;
				break;
			case NodeType::OpenDoubleBracket:
				sp::push_value(parameters, ParamInfo{end_pos, NodeType::CloseDoubleBracket});
				prec_offset += 32;
				break;
			default:
				raise_error("unrecognized value node", token->pos);
		}
		sp::push_value(nodes, op_node);
		++token;
		if (sp::back(precs)[0] < unary_prec){
			[[unlikely]] if (sp::is_full(precs))
				raise_error("exceeded expression depth", op_node.pos);
			sp::push_value(precs, (uint32_t [2]){unary_prec, end_pos});
		}
		continue;
	Finish:
		++token;

		for (;;){
			size_t encloser_index = (size_t)token->type - (size_t)NodeType::ClosePar;
			if (encloser_index > 3) break;
			
			const char *encloser_name = (const char *[]){
				"parenthesis", "braces", "square brackets", "double square brackets",
			}[encloser_index];

			[[unlikely]] if (token->type != sp::back(parameters).finisher)
				raise_error(
						"unmatched pair of ", encloser_name,
						nodes[sp::back(parameters).index].pos
				);
			sp::pop(parameters);
			prec_offset -= 32;
			[[unlikely]] if ((int32_t)prec_offset < 0)
				raise_error("too many closing ", encloser_name, token->pos);
			++token;
		}


		op_node= *token;
		++token;

		size_t prec_index = (size_t)op_node.type - (size_t)NodeType::Comma;
		if (prec_index >= sp::len(prec_table)) return nodes; 

		uint32_t prec = prec_table[prec_index] + prec_offset;
	
		if (prec <= sp::back(precs)[0]){
			while (prec <= precs[sp::len(precs)-2][0]) sp::pop(precs);
			sp::back(precs)[0] = prec;
			goto NoPrecChange;
		}

		[[unlikely]] if (sp::is_full(precs))
			raise_error("exceeded expression depth", op_node.pos);
		sp::push_value(precs, (uint32_t [2]){prec-right_to_left(op_node.type), end_pos});
	
	NoPrecChange:
		switch (op_node.type){
		case NodeType::OpenPar:
		case NodeType::OpenBrace:
		case NodeType::OpenBracket:{
				NodeType finisher_type = (NodeType)(
					(uint16_t)op_node.type + ((uint16_t)NodeType::ClosePar - (uint16_t)NodeType::OpenPar)
				);
				bool has_arguments = token->type != finisher_type; 
				op_node.u16 = has_arguments;	
				if (!has_arguments){
					sp::push_value(nodes, op_node);
					goto Finish;
				}
				sp::push_value(parameters, ParamInfo{sp::back(precs)[1], finisher_type});
				prec_offset += 32;
			} break;
		case NodeType::Comma:
			if (sp::back(parameters).index == UINT32_MAX)
				raise_error("multiple return values are not yet supportred", op_node.pos); // TO DO: add multiple return values
			++nodes[(size_t)sp::back(parameters).index].u16; // TO DO: reair nested function calls
			continue;
		case NodeType::Colon:
		case NodeType::Slice:
			raise_error("not implemented", op_node.pos); // TO DO: implement it
		default:
			break;
		}
		insert(nodes, sp::back(precs)[1], op_node);
	}
}

