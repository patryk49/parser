#include "tokenizer.hpp"

// there are 3 namespace which elements of it cannot collide
// - space of functions
// - space of classes
// - space of variables

// functions and classes can be overloaded
// local variables can shadow other variables
// golbal valiebles cannot collide with each self


uint8_t prec_table[] = {
	1,  1,            // Comma, Slice
	20, 17, 20,       // OpenPar, OpenBrace, OpenBracket,
	20, 17, 20,       // GetProcedure, GetSomethingInBraces, GetField,
	
// LEFT TO RIGHT
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

// RIGHT TO LEFT
	2, 2,             // Assign, ExpandAssign,
	2, 2,             // AddAssign, SubtractAssign,
	2, 2,             // MultiplyAssign, DivideAssign,
	2,                // ModuloAssign,
	2,                // ConcatenateAssign,
	2, 2,             // BitOrAssign, BitNorAssign,
	2, 2,             // BitAndAssign, BitNandAssign,
	2,                // BitXorAssign,
	2, 2,             // LeftShiftAssign, RightShiftAssign, 
	2, 2,             // ArrayAddAssign, ArraySubtractAssign,
	2, 2,             // ArrayMultiplyAssign, ArrayDivideAssign,
	2,                // ArrayModuloAssign,
	2,                // ArrayConcatenateAssign,
	2, 2,             // ArrayBitOrAssign, ArrayBitNorAssign,
	2, 2,             // ArrayBitAndAssign, ArrayBitNandAssign,
	2,                // ArrayBitXorAssign,
	2, 2,             // ArrayLeftShiftAssign, ArrayRightShiftAssign,	
	
	1,                // Ternary,
	16, 16,           // Cast, Reinterpret,
};



SP_CSI size_t right_to_left(NodeType t) noexcept{ return t >= NodeType::Assign; }

template<class T, class A>
SP_CSI void insert(sp::DynamicArray<T, A> &arr, size_t pos, const T &value) noexcept{
	sp::push(arr);
	for (size_t i=sp::len(arr); --i!=pos;) arr[i] = arr[i-1];
	arr[pos] = value;
}



// finisher equal to NodeType::Comma means that expression should be termianted with comma
struct ParamInfo{
	uint32_t index;
	NodeType finisher;
	bool is_list;
	bool expects_value;
};

using NodeArrayType = sp::DynamicArray<Node, sp::MallocAllocator<>>;

Node parse_expression(
	NodeArrayType &nodes,
	const Node **token_iter
) noexcept{ // returns last node
	sp::FiniteArray<uint32_t [2], 32> precs;
	sp::FiniteArray<ParamInfo, 32> context;
	sp::push_value(precs, (uint32_t [2]){0, 0});
	sp::push_value(context, ParamInfo{0, NodeType(0), false, false});
	size_t prec_offset = 0;
	
	const char *ClosingNameTable[] = {
		"parenthesis", "braces", "square brackets", "double square brackets"
 	};
	
	const Node *token = *token_iter;
	Node op_node;
	for (;;){
	Continue:
		size_t end_pos = sp::len(nodes);

		op_node = *token;
		++token;
		
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
			case NodeType::StaticRun:
				unary_prec -= 14 + 3;
			case NodeType::Range:
			case NodeType::ViewRange:
				unary_prec += 3;
			case NodeType::LogicNot:
			case NodeType::BitNot:
			case NodeType::Dereference:
			case NodeType::StaticSize:
			case NodeType::StaticLen:
			case NodeType::Inline:
				break;
			case NodeType::Name:
			case NodeType::Integer:
			case NodeType::Unsigned:
			case NodeType::Float:
			case NodeType::Double:
			case NodeType::Character:
			case NodeType::String:
				sp::push_value(nodes, op_node);
				goto Finish;
			case NodeType::OpenPar:
				sp::push_value(context, ParamInfo{op_node.pos, NodeType::ClosePar, false, false});
				prec_offset += 32;
				continue;
			case NodeType::OpenBrace:
				if (token->type == NodeType::CloseBrace){
					op_node.type = NodeType::EmptyArray;
					sp::push_value(nodes, op_node);
					++token;
					goto Finish;
				}
				sp::push_value(context, ParamInfo{sp::len(nodes), NodeType::CloseBrace, true, false});
				op_node.type = NodeType::ArrayLiteral;
				op_node.data.size = 1;
				prec_offset += 32;
				break;
			case NodeType::OpenBracket:
				sp::push_value(context, ParamInfo{op_node.pos, NodeType::CloseBracket, false, true});
				op_node.type = NodeType::FixedArray;
				prec_offset += 32;
				unary_prec += 4;
				break;
			case NodeType::FiniteArray:
				sp::push_value(
					context, ParamInfo{op_node.pos, NodeType::CloseDoubleBracket, false, true}
				);
				prec_offset += 32;
				unary_prec += 4;
				break;
			case NodeType::Slice:
				[[unlikely]] if (
					sp::back(context).finisher != NodeType::CloseBracket
					|| sp::back(context).expects_value
				){
					raise_error("invalid usage of the slice operator", op_node.pos);
				}
				nodes[sp::back(context).index].type = NodeType::Slice;
				[[unlikely]] if (nodes[sp::back(context).index].data.size != 1)
					raise_error("commas inside the slice expression", op_node.pos);
				if (token->type == NodeType::CloseBracket){
					nodes[sp::back(context).index].data.size = 3;
					sp::pop(context);
					++token;
					prec_offset -= 32;
					goto Finish;
				}
				continue;
			default:
				raise_error("missing value", op_node.pos);
		}
		sp::push_value(nodes, op_node);
		if (sp::back(precs)[0] < unary_prec){
			[[unlikely]] if (sp::is_full(precs))
				raise_error("exceeded expression depth", op_node.pos);
			sp::push_value(precs, (uint32_t [2]){unary_prec, end_pos});
		}
		continue;

	Finish:
		for (;;){
			op_node = *token;
			size_t encloser_index = (size_t)op_node.type - (size_t)NodeType::ClosePar;
			if (encloser_index > sp::len(ClosingNameTable)) break;

			if (sp::len(context) == 1) break;
			[[unlikely]] if (op_node.type != sp::back(context).finisher){
				// handle the case when two single brackets are closed with one double bracket
				if (encloser_index == 3){
					if (
						sp::back(context).finisher == NodeType::CloseBracket
						&& sp::len(context)>1 && context[sp::len(context)-2].finisher==NodeType::CloseBracket
					){
						sp::pop(context);
						prec_offset -= 32;
						goto BreakIf;
					}
				}
				raise_error("too many closing ", ClosingNameTable[encloser_index], op_node.pos);
			}
			if (encloser_index==1 && nodes[sp::back(context).index].type==NodeType::ArrayLiteral){
				if ((token+1)->type == NodeType::Assign){
					++token;
					nodes[sp::back(context).index].type = NodeType::ExpandAssign;
					op_node.type = NodeType::ExpandAssign;
					sp::pop(context);
					prec_offset -= 32;
					goto ExpandAssign;
				}
			}
		BreakIf:
			sp::pop(context);
			prec_offset -= 32;
			++token;
			
			if (sp::end(context)->expects_value) goto Continue;
			op_node = *token;
		}
	ExpandAssign:
		++token;

		size_t prec_index = (size_t)op_node.type - (size_t)NodeType::Comma;
		if (prec_index >= sp::len(prec_table)) goto Return;

		uint32_t prec = prec_table[prec_index] + prec_offset;
	
		if (prec <= sp::back(precs)[0]){
			while (prec <= precs[sp::len(precs)-2][0]) sp::pop(precs);
			sp::back(precs)[0] = prec;
		} else{
			[[unlikely]] if (sp::is_full(precs))
				raise_error("exceeded expression depth", op_node.pos);
			sp::push_value(precs, (uint32_t [2]){prec-right_to_left(op_node.type), end_pos});
		}
		
		switch (op_node.type){
		case NodeType::OpenPar:
		case NodeType::OpenBrace:
		case NodeType::OpenBracket:
		case NodeType::GetProcedure:
		case NodeType::GetSomethingInBraces:
		case NodeType::GetField:{ // TO DO: handle unary postfix operators
				NodeType finisher_type = (NodeType)((uint16_t)op_node.type + (
					op_node.type < NodeType::GetProcedure
					? ((uint16_t)NodeType::ClosePar - (uint16_t)NodeType::OpenPar)
					: ((uint16_t)NodeType::ClosePar - (uint16_t)NodeType::GetProcedure)
				));
				bool has_arguments = token->type != finisher_type; 
				op_node.data.size = has_arguments;	
				if (!has_arguments){
					sp::push_value(nodes, op_node);
					++token;
					goto Finish;
				}
				sp::push_value(context, ParamInfo{sp::back(precs)[1], finisher_type, true, false});
				prec_offset += 32;
			} break;
		case NodeType::Comma:
			if (!sp::back(context).is_list) goto Return;
			++nodes[(size_t)sp::back(context).index].data.size;
			continue;
		case NodeType::ExpandAssign:
			continue;
		case NodeType::Colon:
			raise_error("not implemented", op_node.pos); // TO DO: implement ternary expression
		case NodeType::Slice:
			[[unlikely]] if (
				sp::back(context).finisher != NodeType::CloseBracket || sp::back(context).expects_value
			){
				raise_error("invalid usage of the slice operator", op_node.pos);
			}
			[[unlikely]] if (nodes[sp::back(context).index].data.size != 1)
				raise_error("commas inside the slice expression", op_node.pos);
			nodes[sp::back(context).index].type = NodeType::Slice;
			if (token->type == NodeType::CloseBracket){
				nodes[sp::back(context).index].data.size = 2;
				sp::pop(context);
				++token;
				prec_offset -= 32;
				goto Finish;
			}
			nodes[sp::back(context).index].data.size = 0;
			continue;
		case NodeType::Range: // TO DO: handle it
			op_node.type = NodeType::Slice;
			op_node.data.size = 3;
			break;
		default:
			break;
		}
		insert(nodes, sp::back(precs)[1], op_node);
	}
Return:
	{
		size_t encloser_index = (size_t)op_node.type - (size_t)NodeType::ClosePar;
		[[unlikely]] if (sp::len(context) != 1){
			raise_error(
				"unmatched pair of ",
				ClosingNameTable[(size_t)sp::back(context).finisher - (size_t)NodeType::ClosePar],
				sp::back(context).finisher!=NodeType::ClosePar | sp::back(context).is_list
				? nodes[sp::back(context).index].pos
				: sp::back(context).index
			);
		}
		*token_iter = token;
		return op_node;
	}
}
