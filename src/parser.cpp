/*
 *      Author: Earthcomputer
 */

#include <iostream>
#include <cstdlib>
#include <vector>
#include <map>
#include <set>
#include "crosslang_ast.hpp"
#include "tokenizer.hpp"
#include "parser.hpp"

parser::parser_exception::parser_exception(const char* desc, int pos) :
		pos(pos), desc(desc) {
}
parser::parser_exception::~parser_exception() {
	delete desc;
}
const char* parser::parser_exception::what() {
	return desc;
}
int parser::parser_exception::get_pos() {
	return pos;
}

std::map<std::string, ast::modifier> create_modifiers() {
	std::map<std::string, ast::modifier> modifiers;
	modifiers["global"] = ast::modifier::GLOBAL;
	return modifiers;
}
std::map<std::string, ast::modifier> modifiers = create_modifiers();
std::set<std::string> create_left_unary_operators() {
	std::set<std::string> operators;
	operators.insert("!");
	operators.insert("~");
	operators.insert("+");
	operators.insert("-");
	operators.insert("++");
	operators.insert("--");
	return operators;
}
std::set<std::string> left_unary_operators = create_left_unary_operators();
std::set<std::string> create_right_unary_operators() {
	std::set<std::string> operators;
	operators.insert("++");
	operators.insert("--");
	return operators;
}
std::set<std::string> right_unary_operators = create_right_unary_operators();
std::set<std::string> create_operators() {
	std::set<std::string> operators;
	operators.insert("+");
	operators.insert("-");
	operators.insert("*");
	operators.insert("/");
	operators.insert("%");
	operators.insert("&");
	operators.insert("|");
	operators.insert("^");
	operators.insert(">>");
	operators.insert("<<");
	operators.insert("&&");
	operators.insert("||");
	operators.insert("^^");
	operators.insert("==");
	operators.insert("!=");
	operators.insert("<");
	operators.insert("<=");
	operators.insert(">");
	operators.insert(">=");
	return operators;
}
std::set<std::string> operators = create_operators();
std::set<std::string> create_assignment_operators() {
	std::set<std::string> operators;
	operators.insert("=");
	operators.insert("+=");
	operators.insert("-=");
	operators.insert("*=");
	operators.insert("/=");
	operators.insert("%=");
	operators.insert("&=");
	operators.insert("|=");
	operators.insert("^=");
	operators.insert(">>=");
	operators.insert("<<=");
	return operators;
}
std::set<std::string> assignment_operators = create_assignment_operators();

class parser_cls {
	std::vector<tokenizer::token>* tokens;
	std::vector<tokenizer::token>::size_type next_index = 0;
	std::vector<std::vector<tokenizer::token>::size_type> saved_next_indices;
public:
	parser_cls(std::vector<tokenizer::token>* tokens) :
			tokens(tokens) {
	}
	~parser_cls() {
		delete tokens;
	}
	std::vector<ast::ast_node*>* consume_root() {
		std::vector<ast::ast_node*>* ret = consume_ast_node_list();
		consume_eof();
		return ret;
	}
private:
	std::vector<ast::ast_node*>* consume_ast_node_list() {
		std::vector<ast::ast_node*>* nodes = new std::vector<ast::ast_node*>;
		tokenizer::token* t = next_token();
		while (is_ast_node_token(t)) {
			nodes->push_back(consume_ast_node());
			t = next_token();
		}
		return nodes;
	}
	ast::ast_node* consume_ast_node() {
		tokenizer::token* t = next_token();
		if (is_module_token(t)) {
			return consume_module_node();
		} else if (is_field_token(t)) {
			return consume_field_node();
		} else if (is_function_token(t)) {
			return consume_function_node();
		} else {
			throw parser::parser_exception("Unexpected token", token_pos(t));
		}
	}
	ast::module_node* consume_module_node() {
		tokenizer::token* t;
		consume_token(is_module_token);
		t = next_token();
		std::string namespace_name;
		if (is_identifier(t)) {
			namespace_name = consume_token(is_identifier)->text;
		} else {
			namespace_name = "";
		}
		consume_token(is_open_brace);
		std::vector<ast::ast_node*>* children = consume_ast_node_list();
		consume_token(is_close_brace);
		return new ast::module_node(namespace_name, children);
	}
	ast::field_node* consume_field_node() {
		consume_token(is_field_token);
		std::set<ast::modifier>* modifiers = consume_modifier_list();
		ast::type_ref type = consume_type_ref();
		std::string name = consume_token(is_identifier)->text;
		ast::expression* initialization_expression = nullptr;
		tokenizer::token* t = next_token();
		if (is_equals(t)) {
			consume_token(is_equals);
			initialization_expression = consume_expression();
		}
		consume_end_statement();
		return new ast::field_node(modifiers, type, name,
				initialization_expression);
	}
	ast::function_node* consume_function_node() {
		consume_token(is_function_token);
		std::set<ast::modifier>* modifiers = consume_modifier_list();
		ast::type_ref type = consume_type_ref();
		std::string name = consume_token(is_identifier)->text;
		consume_token(is_open_parenthesis);
		std::vector<ast::field_node*>* parameters = new std::vector<
				ast::field_node*>;
		tokenizer::token* t = next_token();
		while (!is_close_parenthesis(t)) {
			if (!parameters->empty()) {
				consume_token(is_comma);
			}
			std::set<ast::modifier>* modifiers = consume_modifier_list();
			ast::type_ref type = consume_type_ref();
			std::string name = consume_token(is_identifier)->text;
			ast::expression* initialization_expression = nullptr;
			t = next_token();
			if (is_equals(t)) {
				consume_token(is_equals);
				initialization_expression = consume_expression();
				t = next_token();
			}
			parameters->push_back(
					new ast::field_node(modifiers, type, name,
							initialization_expression));
		}
		consume_token(is_close_parenthesis);
		ast::statement* body = consume_statement(false);
		return new ast::function_node(modifiers, type, name, parameters, body);
	}
	std::set<ast::modifier>* consume_modifier_list() {
		std::set<ast::modifier>* modifier_list = new std::set<ast::modifier>;
		tokenizer::token* t = next_token();
		while (is_modifier(t)) {
			consume_token(is_modifier);
			modifier_list->insert(modifiers[t->text]);
			t = next_token();
		}
		return modifier_list;
	}
	ast::type_ref consume_type_ref() {
		std::vector<std::string>* namespaces = new std::vector<std::string>;
		std::string type_name = consume_token(is_identifier)->text;
		std::vector<ast::type_ref>* generic_args =
				new std::vector<ast::type_ref>;
		tokenizer::token* t = next_token();
		while (is_namespace_operator(t)) {
			consume_token(is_namespace_operator);
			namespaces->push_back(type_name);
			type_name = consume_token(is_identifier)->text;
			t = next_token();
		}
		if (is_open_angled_bracket(t)) {
			consume_token(is_open_angled_bracket);
			t = next_token();
			while (!is_close_angled_bracket(t)) {
				if (!generic_args->empty()) {
					consume_token(is_comma);
				}
				generic_args->push_back(consume_type_ref());
				t = next_token();
			}
			consume_token(is_close_angled_bracket);
		}
		return ast::type_ref(namespaces, type_name, generic_args);
	}
	ast::expression* consume_expression() {
		tokenizer::token* t = next_token();
		int initial_pos = t->pos;
		if (is_operator(t)) {
			if (is_open_parenthesis(t)) {
				// if the expression starts with ( it is either a cast or
				// a parenthesized expression
				consume_token(is_open_parenthesis);
				// first we consume whatever is in the parentheses
				void* enclosed;
				bool is_enclosed_type_ref;
				push_saved_state();
				try {
					// first try parsing it as a type ref (for a cast). If we're wrong,
					// we convert later
					ast::type_ref type = consume_type_ref();
					consume_token(is_close_parenthesis);
					enclosed = &type;
					is_enclosed_type_ref = true;
					discard_saved_state();
				} catch (parser::parser_exception& e) {
					// otherwise consume it as an expression
					revert_saved_state();
					enclosed = consume_expression();
					consume_token(is_close_parenthesis);
					is_enclosed_type_ref = false;
				}
				t = next_token();
				// it is a cast if the stuff inside the parentheses could be
				// read as a type ref, and the next thing can be casted (i.e.
				// starts with an identifier or open parenthesis, or starts
				// with a unary operator. Do not accept the unary operators
				// + or - as the user is more likely to have meant the binary
				// version of these operators
				if (((is_left_unary_operator(t) && t->text != "+"
						&& t->text != "-") || is_open_parenthesis(t)
						|| is_identifier(t)) && is_enclosed_type_ref) {
					ast::type_ref type = *static_cast<ast::type_ref*>(enclosed);
					return new ast::cast_expression(type, consume_expression());
				} else {
					// we have a parenthesized expression
					ast::expression* enclosed_expr;
					if (is_enclosed_type_ref) {
						// if the enclosed was interpreted as a type ref, we
						// need to convert it
						ast::type_ref type =
								*static_cast<ast::type_ref*>(enclosed);
						if (!type.get_generic_args()->empty()) {
							throw parser::parser_exception(
									"Unexpected type reference in parenthesized expression",
									initial_pos);
						}
						enclosed_expr = new ast::identifier_expression(
								type.get_type_name());
						std::vector<std::string>* namespaces =
								type.get_namespaces();
						for (std::vector<std::string>::reverse_iterator it =
								namespaces->rbegin(); it != namespaces->rend();
								++it) {
							enclosed_expr = new ast::namespace_expression(*it,
									enclosed_expr);
						}
					} else {
						enclosed_expr = static_cast<ast::expression*>(enclosed);
					}
					ast::parenthesized_expression* expr =
							new ast::parenthesized_expression(enclosed_expr);
					// check for binary operators and right unary operators after
					// the parenthesized expression
					if (is_middle_binary_operator(t)) {
						std::string operator_name = consume_token(
								is_middle_binary_operator)->text;
						ast::expression* rhs = consume_expression();
						return new ast::operator_expression(expr, operator_name,
								rhs);
					} else if (is_right_unary_operator(t)) {
						std::string operator_name = consume_token(
								is_right_unary_operator)->text;
						return new ast::unary_operator_right_expression(expr,
								operator_name);
					} else {
						return expr;
					}
				}
			} else if (is_left_unary_operator(t)) {
				// the expression may instead start with a left unary operator
				std::string operator_name = consume_token(
						is_left_unary_operator)->text;
				ast::expression* operand = consume_expression();
				return new ast::unary_operator_left_expression(operator_name,
						operand);
			}
		} else if (is_number(t)) {
			// if the first token is a number, the expression must start with a
			// constant numerical expression
			consume_token(is_number);
			// obtain a copy of the token's text for us to work on
			std::string text(t->text.c_str());
			ast::radix rad;
			ast::expression* number_expr;
			if (text.find("0x") == 0 || text.find("0X") == 0) {
				rad = ast::radix::HEX;
				text.erase(0, 2);
			} else if (text.find("0b") == 0 || text.find("0B") == 0) {
				rad = ast::radix::BINARY;
				text.erase(0, 2);
			} else if (text.find('0') == 0) {
				if (text != "0") {
					rad = ast::radix::OCTAL;
					text.erase(0, 1);
				} else {
					rad = ast::radix::DECIMAL;
				}
			} else {
				rad = ast::radix::DECIMAL;
			}

			if (text.empty()) {
				throw parser::parser_exception("Invalid number", t->pos);
			}
			// integers don't contain a decimal point or exponent
			if (text.find(".") == std::string::npos
					&& (rad == ast::radix::HEX
							|| (text.find("e") == std::string::npos
									&& text.find("E") == std::string::npos))) {
				int base;
				switch (rad) {
				case ast::radix::BINARY:
					base = 2;
					break;
				case ast::radix::OCTAL:
					base = 8;
					break;
				case ast::radix::DECIMAL:
					base = 10;
					break;
				case ast::radix::HEX:
					base = 16;
					break;
				}
				// this is a good way to check if the entire string was
				// converted to an integer
				char* end_ptr;
				int value = std::strtol(text.c_str(), &end_ptr, base);
				if ((*end_ptr) != '\0') {
					throw parser::parser_exception("Invalid number", t->pos);
				}
				number_expr = new ast::const_integer_expression(value, rad);
			} else {
				// only decimal numbers are allowed for non-integral types
				if (rad != ast::radix::DECIMAL) {
					throw parser::parser_exception(
							"Not allowed non-integer values for non-decimal numbers",
							t->pos);
				}
				// this is a good way to check if the entire string was
				// converted to a double
				char* end_ptr;
				double value = std::strtod(text.c_str(), &end_ptr);
				if ((*end_ptr) != '\0') {
					throw parser::parser_exception("Invalid number", t->pos);
				}
				number_expr = new ast::const_double_expression(value);
			}
			// check for binary operators
			t = next_token();
			if (is_middle_binary_operator(t)) {
				std::string operator_name = consume_token(
						is_middle_binary_operator)->text;
				ast::expression* rhs = consume_expression();
				return new ast::operator_expression(number_expr, operator_name,
						rhs);
			}
			return number_expr;
		} else if (is_identifier(t)) {
			// if it starts with an identifier, it might be a namespace
			// operator, which would require a recursive function to
			// consume the tokens, so ya, here it is
			ast::expression* expr = consume_expression_identifier_part();
			t = next_token();
			// check for binary operators, which would not be allowed as part
			// of a namespace expression
			if (is_middle_binary_operator(t)) {
				std::string operator_name = consume_token(
						is_middle_binary_operator)->text;
				ast::expression* rhs = consume_expression();
				expr = new ast::operator_expression(expr, operator_name, rhs);
			}
			return expr;
		} else if (is_single_quoted_string(t) || is_double_quoted_string(t)) {
			// string expressions are pretty simple
			consume_token();
			std::string text = t->text;
			text = text.substr(1, text.length() - 2);
			ast::expression* expr = new ast::const_string_expression(text);
			// check for binary operators, e.g. concatenation
			t = next_token();
			if (is_middle_binary_operator(t)) {
				std::string operator_name = consume_token(
						is_middle_binary_operator)->text;
				ast::expression* rhs = consume_expression();
				expr = new ast::operator_expression(expr, operator_name, rhs);
			}
			return expr;
		}
		throw parser::parser_exception("Unexpected token - expected expression",
				initial_pos);
	}
	ast::expression* consume_expression_identifier_part() {
		std::string text = consume_token(is_identifier)->text;
		ast::expression* expr;
		// see what's after the identifier
		tokenizer::token* t = next_token();
		if (is_operator(t)) {
			if (is_open_parenthesis(t)) {
				// if it's a ( then it's a call expression
				consume_token(is_open_parenthesis);
				std::vector<ast::expression*>* operands = new std::vector<
						ast::expression*>;
				t = next_token();
				while (!is_close_parenthesis(t)) {
					if (!operands->empty()) {
						consume_token(is_comma);
					}
					operands->push_back(consume_expression());
					t = next_token();
				}
				consume_token(is_close_parenthesis);
				expr = new ast::call_expression(text, operands);
			} else if (is_namespace_operator(t)) {
				// if it's a :: then it's a namespace expression
				consume_token(is_namespace_operator);
				ast::expression* operand = consume_expression_identifier_part();
				expr = new ast::namespace_expression(text, operand);
			} else {
				// otherwise it's just a plain identifier expression
				expr = new ast::identifier_expression(text);
			}
		} else {
			// otherwise it's just a plain identifier expression
			expr = new ast::identifier_expression(text);
		}
		if (expr->is_of_expression_kind(ast::expression_kind::IDENTIFIER)) {
			// if the "identifier expression" is true or false, then it may
			// instead be a boolean expression
			ast::identifier_expression* id =
					static_cast<ast::identifier_expression*>(expr);
			if (id->get_identifier() == "true") {
				delete expr;
				expr = new ast::const_boolean_expression(true);
			} else if (id->get_identifier() == "false") {
				delete expr;
				expr = new ast::const_boolean_expression(false);
			}
		}
		// check for array access expressions
		t = next_token();
		while (is_open_square_bracket(t)) {
			consume_token(is_open_square_bracket);
			std::vector<ast::expression*>* indices = new std::vector<
					ast::expression*>;
			t = next_token();
			while (!is_close_square_bracket(t)) {
				if (!indices->empty()) {
					consume_token(is_comma);
				}
				indices->push_back(consume_expression());
				t = next_token();
			}
			consume_token(is_close_square_bracket);
			expr = new ast::array_expression(expr, indices);
			t = next_token();
		}
		// check for right unary operators
		if (is_right_unary_operator(t)) {
			std::string operator_name =
					consume_token(is_right_unary_operator)->text;
			expr = new ast::unary_operator_right_expression(expr,
					operator_name);
		}
		return expr;
	}
	std::vector<ast::statement*>* consume_statement_list(
			bool (*end_condition)(tokenizer::token*)) {
		std::vector<ast::statement*>* statements = new std::vector<
				ast::statement*>;
		tokenizer::token* t = next_token();
		while (!end_condition(t)) {
			statements->push_back(consume_statement(true));
			t = next_token();
		}
		return statements;
	}
	ast::statement* consume_statement(bool allow_semicolon) {
		tokenizer::token* t = next_token();
		ast::statement* stmt;
		if (is_open_brace(t)) {
			stmt = consume_block_statement();
		} else if (is_if_token(t)) {
			stmt = consume_if_statement();
		} else if (is_while_token(t)) {
			stmt = consume_while_statement();
		} else if (is_do_token(t)) {
			stmt = consume_do_while_statement();
		} else if (is_for_token(t)) {
			stmt = consume_for_statement();
		} else if (is_forever_token(t)) {
			stmt = consume_forever_statement();
		} else if (is_repeat_token(t)) {
			stmt = consume_repeat_statement();
		} else if (is_return_token(t)) {
			stmt = consume_return_statement();
		} else {
			push_saved_state();
			try {
				stmt = consume_variable_declaration_statement();
				discard_saved_state();
			} catch (parser::parser_exception& e) {
				revert_saved_state();
				push_saved_state();
				try {
					stmt = consume_assignment_statement();
					discard_saved_state();
				} catch (parser::parser_exception& e) {
					revert_saved_state();
					stmt = new ast::expression_statement(consume_expression());
				}
			}
		}
		if (allow_semicolon) {
			consume_end_statement();
		}
		return stmt;
	}
	ast::block_statement* consume_block_statement() {
		consume_token(is_open_brace);
		std::vector<ast::statement*>* children = consume_statement_list(
				is_close_brace);
		consume_token(is_close_brace);
		return new ast::block_statement(children);
	}
	ast::variable_declaration_statement* consume_variable_declaration_statement() {
		std::set<ast::modifier> *modifiers = consume_modifier_list();
		ast::type_ref type = consume_type_ref();
		std::string name = consume_token(is_identifier)->text;
		ast::expression* initialization_expression = nullptr;
		tokenizer::token* t = next_token();
		if (is_equals(t)) {
			consume_token(is_equals);
			initialization_expression = consume_expression();
		}
		return new ast::variable_declaration_statement(modifiers, type, name,
				initialization_expression);
	}
	ast::assignment_statement* consume_assignment_statement() {
		ast::expression* lhs = consume_expression();
		std::string operator_name = consume_token(is_assignment_operator)->text;
		ast::expression* rhs = consume_expression();
		return new ast::assignment_statement(lhs, operator_name, rhs);
	}
	ast::if_statement* consume_if_statement() {
		consume_token(is_if_token);
		tokenizer::token* t = next_token();
		bool condition_in_parentheses = is_open_parenthesis(t);
		if (condition_in_parentheses) {
			consume_token(is_open_parenthesis);
		}
		ast::expression* condition = consume_expression();
		if (condition_in_parentheses) {
			consume_token(is_close_parenthesis);
		}
		t = next_token();
		if (is_then_token(t)) {
			consume_token(is_then_token);
		}
		ast::statement* if_clause = consume_statement(false);
		ast::statement* else_clause = nullptr;
		t = next_token();
		if (is_else_token(t)) {
			consume_token(is_else_token);
			else_clause = consume_statement(false);
		}
		return new ast::if_statement(condition, if_clause, else_clause);
	}
	ast::while_statement* consume_while_statement() {
		consume_token(is_while_token);
		tokenizer::token* t = next_token();
		bool condition_in_parentheses = is_open_parenthesis(t);
		if (condition_in_parentheses) {
			consume_token(is_open_parenthesis);
		}
		ast::expression* condition = consume_expression();
		if (condition_in_parentheses) {
			consume_token(is_close_parenthesis);
		}
		ast::statement* while_clause = consume_statement(false);
		return new ast::while_statement(condition, while_clause);
	}
	ast::do_while_statement* consume_do_while_statement() {
		consume_token(is_do_token);
		ast::statement* do_while_clause = consume_statement(false);
		consume_token(is_while_token);
		tokenizer::token* t = next_token();
		bool condition_in_parentheses = is_open_parenthesis(t);
		if (condition_in_parentheses) {
			consume_token(is_open_parenthesis);
		}
		ast::expression* condition = consume_expression();
		if (condition_in_parentheses) {
			consume_token(is_close_parenthesis);
		}
		return new ast::do_while_statement(do_while_clause, condition);
	}
	ast::for_statement* consume_for_statement() {
		consume_token(is_for_token);
		consume_token(is_open_parenthesis);

		tokenizer::token* t = next_token();
		ast::statement* initializer;
		if (is_semicolon(t)) {
			initializer = nullptr;
		} else {
			initializer = consume_statement(false);
		}
		consume_token(is_semicolon);

		t = next_token();
		ast::expression* condition;
		if (is_semicolon(t)) {
			condition = nullptr;
		} else {
			condition = consume_expression();
		}
		consume_token(is_semicolon);

		t = next_token();
		ast::statement* increment;
		if (is_close_parenthesis(t)) {
			increment = nullptr;
		} else {
			increment = consume_statement(false);
		}
		consume_token(is_close_parenthesis);

		ast::statement* for_clause = consume_statement(false);
		return new ast::for_statement(initializer, condition, increment,
				for_clause);
	}
	ast::forever_statement* consume_forever_statement() {
		consume_token(is_forever_token);
		ast::statement* forever_clause = consume_statement(false);
		return new ast::forever_statement(forever_clause);
	}
	ast::repeat_statement* consume_repeat_statement() {
		consume_token(is_repeat_token);
		tokenizer::token* t = next_token();
		bool times_in_parentheses = is_open_parenthesis(t);
		if (times_in_parentheses) {
			consume_token(is_open_parenthesis);
		}
		ast::expression* times = consume_expression();
		if (times_in_parentheses) {
			consume_token(is_close_parenthesis);
		}
		ast::statement* repeat_clause = consume_statement(false);
		return new ast::repeat_statement(times, repeat_clause);
	}
	ast::return_statement* consume_return_statement() {
		consume_token(is_return_token);
		ast::expression* operand = consume_expression();
		return new ast::return_statement(operand);
	}
	void consume_end_statement() {
		if (is_semicolon(next_token())) {
			consume_token(is_semicolon);
		}
	}

	static bool is_identifier(tokenizer::token* t) {
		return t != nullptr && t->kind == tokenizer::token_kind::IDENTIFIER;
	}
	static bool is_number(tokenizer::token* t) {
		return t != nullptr && t->kind == tokenizer::token_kind::NUMBER;
	}
	static bool is_operator(tokenizer::token* t) {
		return t != nullptr && t->kind == tokenizer::token_kind::OPERATOR;
	}
	static bool is_single_quoted_string(tokenizer::token* t) {
		return t != nullptr
				&& t->kind == tokenizer::token_kind::SINGLE_QUOTED_STRING;
	}
	static bool is_double_quoted_string(tokenizer::token* t) {
		return t != nullptr
				&& t->kind == tokenizer::token_kind::DOUBLE_QUOTED_STRING;
	}

	static bool is_ast_node_token(tokenizer::token* t) {
		return is_field_token(t) || is_function_token(t) || is_module_token(t);
	}
	static bool is_field_token(tokenizer::token* t) {
		return is_identifier(t) && (t->text == "fd" || t->text == "field");
	}
	static bool is_function_token(tokenizer::token* t) {
		return is_identifier(t) && (t->text == "fn" || t->text == "function");
	}
	static bool is_module_token(tokenizer::token* t) {
		return is_identifier(t) && (t->text == "md" || t->text == "module");
	}
	static bool is_if_token(tokenizer::token* t) {
		return is_identifier(t) && t->text == "if";
	}
	static bool is_then_token(tokenizer::token* t) {
		return is_identifier(t) && t->text == "then";
	}
	static bool is_else_token(tokenizer::token* t) {
		return is_identifier(t) && t->text == "else";
	}
	static bool is_while_token(tokenizer::token* t) {
		return is_identifier(t) && t->text == "while";
	}
	static bool is_do_token(tokenizer::token* t) {
		return is_identifier(t) && t->text == "do";
	}
	static bool is_for_token(tokenizer::token* t) {
		return is_identifier(t) && t->text == "for";
	}
	static bool is_forever_token(tokenizer::token* t) {
		return is_identifier(t) && t->text == "forever";
	}
	static bool is_repeat_token(tokenizer::token* t) {
		return is_identifier(t) && t->text == "repeat";
	}
	static bool is_return_token(tokenizer::token* t) {
		return is_identifier(t) && t->text == "return";
	}
	static bool is_modifier(tokenizer::token* t) {
		return is_identifier(t) && modifiers.count(t->text);
	}
	static bool is_open_brace(tokenizer::token* t) {
		return is_operator(t) && t->text == "{";
	}
	static bool is_close_brace(tokenizer::token* t) {
		return is_operator(t) && t->text == "}";
	}
	static bool is_open_parenthesis(tokenizer::token* t) {
		return is_operator(t) && t->text == "(";
	}
	static bool is_close_parenthesis(tokenizer::token* t) {
		return is_operator(t) && t->text == ")";
	}
	static bool is_open_square_bracket(tokenizer::token* t) {
		return is_operator(t) && t->text == "[";
	}
	static bool is_close_square_bracket(tokenizer::token* t) {
		return is_operator(t) && t->text == "]";
	}
	static bool is_open_angled_bracket(tokenizer::token* t) {
		return is_operator(t) && t->text == "<";
	}
	static bool is_close_angled_bracket(tokenizer::token* t) {
		return is_operator(t) && t->text == ">";
	}
	static bool is_equals(tokenizer::token* t) {
		return is_operator(t) && t->text == "=";
	}
	static bool is_semicolon(tokenizer::token* t) {
		return is_operator(t) && t->text == ";";
	}
	static bool is_comma(tokenizer::token* t) {
		return is_operator(t) && t->text == ",";
	}
	static bool is_namespace_operator(tokenizer::token* t) {
		return is_operator(t) && t->text == "::";
	}
	static bool is_left_unary_operator(tokenizer::token* t) {
		return is_operator(t)
				&& left_unary_operators.find(t->text)
						!= left_unary_operators.end();
	}
	static bool is_right_unary_operator(tokenizer::token* t) {
		return is_operator(t)
				&& right_unary_operators.find(t->text)
						!= right_unary_operators.end();
	}
	static bool is_middle_binary_operator(tokenizer::token* t) {
		return is_operator(t) && operators.find(t->text) != operators.end();
	}
	static bool is_assignment_operator(tokenizer::token* t) {
		return is_operator(t)
				&& assignment_operators.find(t->text)
						!= assignment_operators.end();
	}

	tokenizer::token* next_token() {
		if (next_index >= tokens->size()) {
			return nullptr;
		} else {
			return &((*tokens)[next_index]);
		}
	}
	tokenizer::token_kind next_token_kind() {
		tokenizer::token* token = next_token();
		if (token == nullptr) {
			return tokenizer::token_kind::END_OF_FILE;
		} else {
			return token->kind;
		}
	}
	tokenizer::token* consume_token() {
		tokenizer::token* ret = next_token();
		next_index++;
		return ret;
	}
	tokenizer::token* consume_token(bool (*filter)(tokenizer::token*)) {
		tokenizer::token* t = consume_token();
		if (t == nullptr || !filter(t)) {
			throw parser::parser_exception("Unexpected token", token_pos(t));
		}
		return t;
	}
	void push_saved_state() {
		saved_next_indices.push_back(next_index);
	}
	void discard_saved_state() {
		if (saved_next_indices.empty()) {
			std::cerr << "State has not been saved!" << std::endl;
		}
		saved_next_indices.pop_back();
	}
	void revert_saved_state() {
		if (saved_next_indices.empty()) {
			std::cerr << "State has not been saved!" << std::endl;
			throw std::exception();
		}
		next_index = saved_next_indices[saved_next_indices.size() - 1];
		saved_next_indices.pop_back();
	}
	void consume_eof() {
		tokenizer::token* t = consume_token();
		if (t != nullptr) {
			throw parser::parser_exception("Expected end of file", t->pos);
		}
	}
	int token_pos(tokenizer::token* t) {
		return t == nullptr ? -1 : t->pos;
	}
};

enum class node_type {
	AST_NODE, STATEMENT, EXPRESSION
};

class parentifier_visitor: public ast::ast_visitor {
	std::vector<void*> parent_stack;
	std::vector<node_type> types;
public:
	void visit_ast_node(ast::ast_node* node) {
		if (!parent_stack.empty()) {
			node->set_parent_node(
					static_cast<ast::ast_node*>(parent_stack.back()));
		}
		parent_stack.push_back(node);
		types.push_back(node_type::AST_NODE);
		ast::ast_visitor::visit_ast_node(node);
		parent_stack.pop_back();
		types.pop_back();
	}
	void visit_statement(ast::statement* stmt) {
		void* parent = parent_stack.back();
		node_type type = types.back();
		if (type == node_type::STATEMENT) {
			stmt->set_parent_statement(static_cast<ast::statement*>(parent));
		} else {
			stmt->set_parent_node(static_cast<ast::ast_node*>(parent));
		}
		parent_stack.push_back(stmt);
		types.push_back(node_type::STATEMENT);
		ast::ast_visitor::visit_statement(stmt);
		parent_stack.pop_back();
		types.pop_back();
	}
	void visit_expression(ast::expression* expr) {
		void* parent = parent_stack.back();
		node_type type = types.back();
		if (type == node_type::EXPRESSION) {
			expr->set_parent_expression(static_cast<ast::expression*>(parent));
		} else if (type == node_type::STATEMENT) {
			expr->set_parent_statement(static_cast<ast::statement*>(parent));
		} else {
			expr->set_parent_node(static_cast<ast::ast_node*>(parent));
		}
		parent_stack.push_back(expr);
		types.push_back(node_type::EXPRESSION);
		ast::ast_visitor::visit_expression(expr);
		parent_stack.pop_back();
		types.pop_back();
	}
};

std::vector<std::set<std::string>> create_op_precedence() {
	std::vector<std::set<std::string>> levels;
	{
		std::set<std::string> level;
		level.insert("*");
		level.insert("/");
		level.insert("%");
		levels.push_back(level);
	}
	{
		std::set<std::string> level;
		level.insert("+");
		level.insert("-");
		levels.push_back(level);
	}
	{
		std::set<std::string> level;
		level.insert("==");
		level.insert("!=");
		level.insert("<");
		level.insert("<=");
		level.insert(">");
		level.insert(">=");
		levels.push_back(level);
	}
	{
		std::set<std::string> level;
		level.insert("&&");
		level.insert("||");
		level.insert("^^");
		levels.push_back(level);
	}
	{
		std::set<std::string> level;
		level.insert("&");
		level.insert("|");
		level.insert("^");
		level.insert(">>");
		level.insert("<<");
		levels.push_back(level);
	}
	return levels;
}
std::vector<std::set<std::string>> op_precedence = create_op_precedence();

class operator_precedence_fix_visitor: public ast::ast_visitor {
	// by default, 1 * 2 - 3 / 4 is encoded as (1 * (2 - (3 / 4)))
	// this is changed by this visitor into    ((1 * 2) - (3 / 4))
	std::set<ast::operator_expression*> already_visited;
public:
	void visit_operator_expression(ast::operator_expression* expr) {
		if (already_visited.find(expr) != already_visited.end()) {
			return;
		}
		already_visited.insert(expr);
		ast::expression* parent = expr->get_parent_expression();
		if (parent == nullptr
				|| !parent->is_of_expression_kind(
						ast::expression_kind::OPERATOR)) {
			return;
		}
		ast::operator_expression* parent_op =
				static_cast<ast::operator_expression*>(parent);

		int this_op_precedence = 0;
		for (std::set<std::string>& level : op_precedence) {
			if (level.find(expr->get_operator()) != level.end()) {
				break;
			}
			this_op_precedence++;
		}

		int parent_op_precedence = 0;
		for (std::set<std::string>& level : op_precedence) {
			if (level.find(parent_op->get_operator()) != level.end()) {
				break;
			}
			parent_op_precedence++;
		}

		if (parent_op_precedence <= this_op_precedence) {
			// precedence in wrong order, change it
			parent_op->set_rhs(expr->get_lhs());
			expr->set_lhs(parent_op);
			std::vector<ast::expression**>* parent_parent_children;
			if (parent_op->get_parent_expression() != nullptr) {
				expr->set_parent_expression(parent_op->get_parent_expression());
				parent_parent_children =
						parent_op->get_parent_expression()->get_children();
			} else if (parent_op->get_parent_statement() != nullptr) {
				expr->set_parent_expression(nullptr);
				expr->set_parent_statement(parent_op->get_parent_statement());
				parent_parent_children =
						parent_op->get_parent_statement()->get_child_expressions();
				parent_op->set_parent_statement(nullptr);
			} else {
				expr->set_parent_expression(nullptr);
				expr->set_parent_node(parent_op->get_parent_node());
				parent_parent_children =
						parent_op->get_parent_node()->get_child_expressions();
				parent_op->set_parent_node(nullptr);
			}
			parent_op->set_parent_expression(expr);
			for (ast::expression**& child_ptr : *parent_parent_children) {
				if (*child_ptr == parent_op) {
					*child_ptr = expr;
					break;
				}
			}
			delete parent_parent_children;
		}
	}
};
std::vector<ast::ast_node*>* parser::parse(
		std::vector<tokenizer::token> tokens) {
	parser_cls p(&tokens);
	std::vector<ast::ast_node*>* nodes = p.consume_root();
	ast::ast_visitor* visitor = new parentifier_visitor;
	visitor->visit_all(nodes);
	delete visitor;
	visitor = new operator_precedence_fix_visitor;
	visitor->visit_all(nodes);
	delete visitor;
	return nodes;
}

