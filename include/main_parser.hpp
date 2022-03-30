#include "expr_parser.hpp"

struct FunctionModel{
	uint32_t args = 0;
	uint32_t ast;
	uint32_t output;
	uint32_t labels;
	uint8_t label_count = 0;
	uint8_t arg_count = 0;
	bool is_inline;
};

struct LabelInfo{
	uint32_t name;
	uint32_t name_len;
	uint32_t index;
};

using LabelArrayType = sp::DynamicArray<LabelInfo, sp::MallocAllocator<>>;



void parse_function(
	NodeArrayType &nodes,
	LabelArrayType &labels,
	const Node **token_iter
) noexcept{ // returns last node
	size_t scope_count = 0;
	const Node *token = *token_iter;
	Node curr = *token;

	FunctionModel model;
	uint8_t arg_count;
	model.labels = sp::len(labels);

	model.is_inline = curr.type == NodeType::Slice;
	token += model.is_inline;
	model.args = sp::len(nodes);

	[[unlikely]] if (token->type != NodeType::OpenPar)
		raise_error("missing parenhessis for function's parameters", token->pos);
	++token;
	curr = *token;
	++token;
	if (curr.type == NodeType::ClosePar) goto ParseReturnType;

	for (;;){ // parse parameters
		++model.arg_count;
		[[unlikely]] if (curr.type != NodeType::Name)
			raise_error("missing function's parameter name", curr.pos);
		switch (token->type){
		case NodeType::Colon:                // pass by value
		case NodeType::DoubleColon:          // require constant parameter
		case NodeType::TripleColon:          // const parameter if possible
		case NodeType::LogicAnd:             // pass by reference
		case NodeType::DoubleColonAmpersand: // const parameter if possible, otherwise pass by reference
		case NodeType::Constant:             // parameter is a specific value
			curr.type = token->type;	
			break;
		default:
			raise_error("expected parameter specification symbol", token->pos);
		}
		sp::push_value(nodes, curr);
		++token;
		size_t start_index = sp::len(nodes);
		curr = parse_expression(nodes, &token);
		if (nodes[start_index].type == NodeType::Assign)
			raise_error("default arguments are not yet supported", nodes[start_index].pos);
	 	
		if (curr.type != NodeType::Comma) break;
		curr = *token;
		++token;
	}
	
	if (curr.type != NodeType::ClosePar)
		[[unlikely]] raise_error("missing closing parenthesis", curr.pos);

ParseReturnType:
	if (token->type != NodeType::OpenScope)
		[[unlikely]] raise_error("missing function's body", token->pos);
	++token;

	model.ast = sp::len(nodes);
	model.labels = sp::len(labels);
	for (;;){
		curr = *token;
		++token;
		uint32_t last_scope_pos = (token-1)->pos;

		switch (curr.type){
			case NodeType::OpenScope:
				last_scope_pos = curr.pos;
				++scope_count;
				break;
			case NodeType::CloseBrace:
				if (!scope_count) goto Return;
				--scope_count;
				break;
			case NodeType::If:

			case NodeType::Greater:
				if (token->type!=NodeType::Name || (token+1)->type!=NodeType::Greater)
					raise_error("wrong label syntax", curr.pos);
				sp::push_value(labels, LabelInfo{
					token->data.index, (uint32_t)token->u16, sp::len(nodes)
				});
				++model.label_count;
				token += 2;
				continue;
			case NodeType::Set:
				sp::push_value(nodes, curr);
				goto ParseExpression;
			case NodeType::Unset:
				if (token->type != NodeType::Name)
					raise_error("missing name of constant", token->pos);
				curr.data.index = token->data.index;
				curr.u16 = token->u16;
				++token;
				if (token->type != NodeType::Terminator)
					raise_error("missing semicolon", token->pos);
				++token;
				break;
			case NodeType::StaticAssert:
				sp::push_value(nodes, curr);
				curr = parse_expression(nodes, &token);
				if (curr.type == NodeType::Comma) goto ParseExpression;
				[[unlikely]] if (curr.type != NodeType::Terminator)
					raise_error("missing semicolon", token->pos);
				curr.type = NodeType::String;
				curr.u16 = 0;
				break;
			case NodeType::Asm:
				// TO DO: implement it
				raise_error("inline assembly is not yet implemented", curr.pos);
			case NodeType::Goto:
				if (token->type != NodeType::Name){
					[[unlikely]] if (token->type != NodeType::OpenPar)
						raise_error("missing name of label after goto", curr.pos);
					curr.type = NodeType::GotoInstruction;
					sp::push_value(nodes, curr);
					goto ParseExpression;
				}
				curr.data.index = token->data.index;
				curr.u16 = token->u16;
				++token;
				[[unlikely]] if (token->type != NodeType::Terminator)
					raise_error("missing semicolon", curr.pos);
				++token;
				break;
			case NodeType::Break:
			case NodeType::Continue:
				curr.data.size = 1;
				if (token->type == NodeType::Name){
					curr.type = (NodeType)(
						(uint16_t)curr.type
						+ ((uint16_t)NodeType::BreakIterator - (uint16_t)NodeType::Break)
					);
					curr.data.index = token->data.index;
					curr.u16 = token->u16;
					++token;
				} else if (token->type == NodeType::Integer){
					curr.data.size = token->data.u64;
					++token;
				}
				[[unlikely]] if (token->type != NodeType::Terminator)
					raise_error("missing semicolon", curr.pos);
				++token;
				break;
			case NodeType::Return:
				sp::push_value(nodes, curr);
				curr = parse_expression(nodes, &token);
				[[unlikely]] if (curr.type != NodeType::Terminator)
					raise_error("missing semicolon", curr.pos);
				continue;
			case NodeType::Name:
				switch (token->type){
				case NodeType::Comma:
					{
						size_t decl_index = sp::len(nodes);
						sp::push_value(nodes, *token);
						nodes[decl_index].data.size = 1;
						sp::push_value(nodes, curr);
						
						for (;;){
							++token;
							if (token->type != NodeType::Name)
								raise_error("missing variable name", token->pos);
							sp::push_value(nodes, *token);
							++token;

							switch (token->type){
							case NodeType::Comma:
								++nodes[decl_index].data.size;
								break;
							case NodeType::Colon:
								nodes[decl_index].type = NodeType::Colon;
								nodes[decl_index].pos = token->pos;
								++token;
								goto ParseExpression;
							default:
								raise_error("expected declaration symbol", token->pos);
							}
						}
					}
				case NodeType::Variable:
				case NodeType::Constant:
				case NodeType::Colon:
				case NodeType::DoubleColon:
					sp::push_value(nodes, *token);
					sp::back(nodes).data.size = 0;
					sp::push_value(nodes, curr);
					token += 2;
				default:;
				}
			default:
				--token;
			ParseExpression:
				{
					size_t start_index = sp::len(nodes);
					curr = parse_expression(nodes, &token);
					if (curr.type != NodeType::Terminator){
						[[unlikely]] if (
							nodes[start_index].type != NodeType::ArrayLiteral
							|| (curr.type!=NodeType::Variable && curr.type!=NodeType::Constant)
						) raise_error("missing semicolon", curr.pos);
						
						curr.data.size = nodes[start_index].data.size;
						curr.type = (NodeType)((uint16_t)curr.type + 2);
						nodes[start_index] = curr;
						curr = parse_expression(nodes, &token);
						[[unlikely]] if (curr.type != NodeType::Terminator)
							raise_error("missing semicolon", curr.pos);
					}
					continue;
				}
		}
		sp::push_value(nodes, curr);
	}
Return:
	curr.type = NodeType::Terminator;
	sp::push_value(nodes, curr);
	*token_iter = token;
}
