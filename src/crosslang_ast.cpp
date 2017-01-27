/*
 *      Author: Earthcomputer
 */

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include "crosslang_ast.hpp"

ast::type_ref::type_ref(std::string type_name) :
		namespaces(), type_name(type_name), generic_args() {
}

ast::type_ref::type_ref(std::vector<std::string>* namespaces,
		std::string type_name) :
		namespaces(namespaces), type_name(type_name), generic_args() {
}
ast::type_ref::type_ref(std::vector<std::string>* namespaces,
		std::string type_name, std::vector<ast::type_ref>* generic_args) :
		namespaces(namespaces), type_name(type_name), generic_args(generic_args) {
}
std::vector<std::string>* ast::type_ref::get_namespaces() {
	return namespaces;
}
std::string ast::type_ref::get_type_name() {
	return type_name;
}
void ast::type_ref::set_type_name(std::string type_name) {
	this->type_name = type_name;
}
std::vector<ast::type_ref>* ast::type_ref::get_generic_args() {
	return generic_args;
}
bool ast::type_ref::is_bool() {
	return namespaces->empty() && generic_args->empty()
			&& (type_name == "bool" || type_name == "boolean");
}
bool ast::type_ref::is_char() {
	return namespaces->empty() && generic_args->empty() && type_name == "char";
}
bool ast::type_ref::is_double() {
	return namespaces->empty() && generic_args->empty() && type_name == "double";
}
bool ast::type_ref::is_float() {
	return namespaces->empty() && generic_args->empty() && type_name == "float";
}
bool ast::type_ref::is_int() {
	return namespaces->empty() && generic_args->empty() && type_name == "int";
}
bool ast::type_ref::is_long() {
	return namespaces->empty() && generic_args->empty() && type_name == "long";
}
bool ast::type_ref::is_short() {
	return namespaces->empty() && generic_args->empty() && type_name == "short";
}
std::string ast::type_ref::to_string() {
	std::string ret = "";
	for (std::string& ns : *namespaces) {
		ret += ns;
		ret += "::";
	}
	ret += type_name;
	if (!generic_args->empty()) {
		ret += "<";
		bool is_first = true;
		for (type_ref& generic_arg : *generic_args) {
			if (!is_first) {
				ret += ", ";
			}
			ret += generic_arg.to_string();
			is_first = false;
		}
		ret += ">";
	}
	return ret;
}
bool ast::type_ref::operator ==(ast::type_ref other) {
	if (namespaces != other.namespaces || type_name != other.type_name) {
		return false;
	}
	if (generic_args->size() != other.generic_args->size()) {
		return false;
	}
	for (int i = 0, e = generic_args->size(); i < e; i++) {
		if ((*generic_args)[i] != (*other.generic_args)[i]) {
			return false;
		}
	}
	return true;
}
bool ast::type_ref::operator !=(ast::type_ref other) {
	return !operator==(other);
}

ast::expression::expression(ast::expression_kind kind) :
		kind(kind) {
}
ast::expression::~expression() {
}
ast::expression_kind ast::expression::get_expression_kind() {
	return kind;
}
bool ast::expression::is_of_expression_kind(ast::expression_kind kind) {
	return this->kind == kind;
}
ast::expression* ast::expression::get_parent_expression() {
	return parent_expression;
}
void ast::expression::set_parent_expression(ast::expression* parent) {
	parent_expression = parent;
}
ast::statement* ast::expression::get_parent_statement() {
	return parent_statement;
}
void ast::expression::set_parent_statement(ast::statement* parent) {
	parent_statement = parent;
}
ast::ast_node* ast::expression::get_parent_node() {
	return parent_node;
}
void ast::expression::set_parent_node(ast::ast_node* parent) {
	parent_node = parent;
}
std::string ast::expression::to_string() {
	std::cerr << "Called expression::to_string()!" << std::endl;
	throw std::exception();
}
std::vector<ast::expression**>* ast::expression::get_children() {
	return new std::vector<ast::expression**>(0);
}
void ast::expression::accept(ast::ast_visitor* visitor) {
	std::cerr << "Called expression::accept(ast_visitor*)!" << std::endl;
	throw std::exception();
}

ast::identifier_expression::identifier_expression(std::string identifier) :
		expression(ast::expression_kind::IDENTIFIER), identifier(identifier) {
}
std::string ast::identifier_expression::get_identifier() {
	return identifier;
}
void ast::identifier_expression::set_identifier(std::string identifier) {
	this->identifier = identifier;
}
std::string ast::identifier_expression::to_string() {
	return "id_expr{" + identifier + "}";
}
void ast::identifier_expression::accept(ast::ast_visitor* visitor) {
	visitor->visit_identifier_expression(this);
}

ast::parenthesized_expression::parenthesized_expression(ast::expression* child) :
		expression(ast::expression_kind::PARENTHESIZED), child(child) {
}
ast::parenthesized_expression::~parenthesized_expression() {
	delete child;
}
ast::expression* ast::parenthesized_expression::get_child() {
	return child;
}
void ast::parenthesized_expression::set_child(ast::expression* child) {
	this->child = child;
}
std::string ast::parenthesized_expression::to_string() {
	return "par_expr{" + child->to_string() + "}";
}
std::vector<ast::expression**>* ast::parenthesized_expression::get_children() {
	return new std::vector<ast::expression**>(1, &child);
}
void ast::parenthesized_expression::accept(ast::ast_visitor* visitor) {
	visitor->visit_parenthesized_expression(this);
}

ast::call_expression::call_expression(std::string name,
		std::vector<ast::expression*>* operands) :
		expression(ast::expression_kind::CALL), name(name), operands(operands) {
}
ast::call_expression::~call_expression() {
	for (ast::expression*& operand : *operands) {
		delete operand;
	}
	delete operands;
}
std::string ast::call_expression::get_name() {
	return name;
}
void ast::call_expression::set_name(std::string name) {
	this->name = name;
}
std::vector<ast::expression*>* ast::call_expression::get_operands() {
	return operands;
}
std::string ast::call_expression::to_string() {
	std::string ret = "call_expr{" + name + " with: ";
	bool is_first = true;
	for (ast::expression*& operand : *operands) {
		if (!is_first) {
			ret += ", ";
		}
		ret += operand->to_string();
		is_first = false;
	}
	ret += "}";
	return ret;
}
std::vector<ast::expression**>* ast::call_expression::get_children() {
	std::vector<ast::expression**>* children =
			new std::vector<ast::expression**>;
	for (ast::expression*& operand : *operands) {
		children->push_back(&operand);
	}
	return children;
}
void ast::call_expression::accept(ast::ast_visitor* visitor) {
	visitor->visit_call_expression(this);
}

ast::namespace_expression::namespace_expression(std::string namespace_name,
		ast::expression* operand) :
		expression(ast::expression_kind::NAMESPACE), namespace_name(
				namespace_name), operand(operand) {
}
ast::namespace_expression::~namespace_expression() {
	delete operand;
}
std::string ast::namespace_expression::get_namespace() {
	return namespace_name;
}
void ast::namespace_expression::set_namespace(std::string namespace_name) {
	this->namespace_name = namespace_name;
}
ast::expression* ast::namespace_expression::get_operand() {
	return operand;
}
void ast::namespace_expression::set_operand(ast::expression* operand) {
	this->operand = operand;
}
std::string ast::namespace_expression::to_string() {
	return "ns_expr{" + namespace_name + "::" + operand->to_string() + "}";
}
std::vector<ast::expression**>* ast::namespace_expression::get_children() {
	return new std::vector<ast::expression**>(1, &operand);
}
void ast::namespace_expression::accept(ast::ast_visitor* visitor) {
	visitor->visit_namespace_expression(this);
}

ast::operator_expression::operator_expression(ast::expression* lhs,
		std::string operator_name, ast::expression* rhs) :
		expression(ast::expression_kind::OPERATOR), lhs(lhs), operator_name(
				operator_name), rhs(rhs) {
}
ast::operator_expression::~operator_expression() {
	delete lhs;
	delete rhs;
}
ast::expression* ast::operator_expression::get_lhs() {
	return lhs;
}
void ast::operator_expression::set_lhs(ast::expression* lhs) {
	this->lhs = lhs;
}
std::string ast::operator_expression::get_operator() {
	return operator_name;
}
void ast::operator_expression::set_operator(std::string operator_name) {
	this->operator_name = operator_name;
}
ast::expression* ast::operator_expression::get_rhs() {
	return rhs;
}
void ast::operator_expression::set_rhs(ast::expression* rhs) {
	this->rhs = rhs;
}
std::string ast::operator_expression::to_string() {
	return "op_expr{" + lhs->to_string() + " " + operator_name + " "
			+ rhs->to_string() + "}";
}
std::vector<ast::expression**>* ast::operator_expression::get_children() {
	std::vector<ast::expression**>* children =
			new std::vector<ast::expression**>(2);
	(*children)[0] = &lhs;
	(*children)[1] = &rhs;
	return children;
}
void ast::operator_expression::accept(ast::ast_visitor* visitor) {
	visitor->visit_operator_expression(this);
}

ast::unary_operator_left_expression::unary_operator_left_expression(
		std::string operator_name, ast::expression* operand) :
		expression(expression_kind::UNARY_OPERATOR_LEFT), operator_name(
				operator_name), operand(operand) {
}
ast::unary_operator_left_expression::~unary_operator_left_expression() {
	delete operand;
}
std::string ast::unary_operator_left_expression::get_operator() {
	return operator_name;
}
void ast::unary_operator_left_expression::set_operator(
		std::string operator_name) {
	this->operator_name = operator_name;
}
ast::expression* ast::unary_operator_left_expression::get_operand() {
	return operand;
}
void ast::unary_operator_left_expression::set_operand(
		ast::expression* operand) {
	this->operand = operand;
}
std::string ast::unary_operator_left_expression::to_string() {
	return "unlop_expr{" + operator_name + " " + operand->to_string() + "}";
}
std::vector<ast::expression**>* ast::unary_operator_left_expression::get_children() {
	return new std::vector<ast::expression**>(1, &operand);
}
void ast::unary_operator_left_expression::accept(ast::ast_visitor* visitor) {
	visitor->visit_unary_operator_left_expression(this);
}

ast::unary_operator_right_expression::unary_operator_right_expression(
		ast::expression* operand, std::string operator_name) :
		expression(ast::expression_kind::UNARY_OPERATOR_RIGHT), operand(
				operand), operator_name(operator_name) {
}
ast::unary_operator_right_expression::~unary_operator_right_expression() {
	delete operand;
}
ast::expression* ast::unary_operator_right_expression::get_operand() {
	return operand;
}
void ast::unary_operator_right_expression::set_operand(
		ast::expression* operand) {
	this->operand = operand;
}
std::string ast::unary_operator_right_expression::get_operator() {
	return operator_name;
}
void ast::unary_operator_right_expression::set_operator(
		std::string operator_name) {
	this->operator_name = operator_name;
}
std::string ast::unary_operator_right_expression::to_string() {
	return "unrop_expr{" + operand->to_string() + " " + operator_name + "}";
}
std::vector<ast::expression**>* ast::unary_operator_right_expression::get_children() {
	return new std::vector<ast::expression**>(1, &operand);
}
void ast::unary_operator_right_expression::accept(ast::ast_visitor* visitor) {
	visitor->visit_unary_operator_right_expression(this);
}

ast::const_boolean_expression::const_boolean_expression(bool value) :
		expression(ast::expression_kind::CONST_BOOLEAN), value(value) {
}
bool ast::const_boolean_expression::get_value() {
	return value;
}
void ast::const_boolean_expression::set_value(bool value) {
	this->value = value;
}
std::string ast::const_boolean_expression::to_string() {
	std::string ret = "bool_expr{";
	if (value) {
		ret += "true";
	} else {
		ret += "false";
	}
	ret += "}";
	return ret;
}
void ast::const_boolean_expression::accept(ast::ast_visitor* visitor) {
	visitor->visit_const_boolean_expression(this);
}

ast::const_integer_expression::const_integer_expression(int value) :
		expression(ast::expression_kind::CONST_INTEGER), value(value), rad(
				radix::DECIMAL) {
}
ast::const_integer_expression::const_integer_expression(int value,
		ast::radix rad) :
		expression(ast::expression_kind::CONST_INTEGER), value(value), rad(rad) {
}
int ast::const_integer_expression::get_value() {
	return value;
}
void ast::const_integer_expression::set_value(int value) {
	this->value = value;
}
ast::radix ast::const_integer_expression::get_radix() {
	return rad;
}
std::string ast::const_integer_expression::to_string() {
	std::ostringstream ret;
	ret << "int_expr{" << value << ", rad=";
	switch (rad) {
	case ast::radix::BINARY:
		ret << "bin";
		break;
	case ast::radix::OCTAL:
		ret << "oct";
		break;
	case ast::radix::DECIMAL:
		ret << "dec";
		break;
	case ast::radix::HEX:
		ret << "hex";
		break;
	}
	ret << "}";
	return ret.str();
}
void ast::const_integer_expression::accept(ast::ast_visitor* visitor) {
	visitor->visit_const_integer_expression(this);
}

ast::const_float_expression::const_float_expression(float value) :
		expression(ast::expression_kind::CONST_FLOAT), value(value) {
}
float ast::const_float_expression::get_value() {
	return value;
}
void ast::const_float_expression::set_value(float value) {
	this->value = value;
}
std::string ast::const_float_expression::to_string() {
	std::ostringstream ret;
	ret << "float_expr{" << value << "}";
	return ret.str();
}
void ast::const_float_expression::accept(ast::ast_visitor* visitor) {
	visitor->visit_const_float_expression(this);
}

ast::const_double_expression::const_double_expression(double value) :
		expression(ast::expression_kind::CONST_DOUBLE), value(value) {
}
double ast::const_double_expression::get_value() {
	return value;
}
void ast::const_double_expression::set_value(double value) {
	this->value = value;
}
std::string ast::const_double_expression::to_string() {
	std::ostringstream ret;
	ret << "double_expr{" << value << "}";
	return ret.str();
}
void ast::const_double_expression::accept(ast::ast_visitor* visitor) {
	visitor->visit_const_double_expression(this);
}

ast::const_string_expression::const_string_expression(std::string value) :
		expression(ast::expression_kind::CONST_STRING), value(value) {
}
std::string ast::const_string_expression::get_value() {
	return value;
}
void ast::const_string_expression::set_value(std::string value) {
	this->value = value;
}
std::string ast::const_string_expression::to_string() {
	return "str_expr{" + value + "}";
}
void ast::const_string_expression::accept(ast::ast_visitor* visitor) {
	visitor->visit_const_string_expression(this);
}

ast::cast_expression::cast_expression(ast::type_ref target_type,
		ast::expression* operand) :
		expression(ast::expression_kind::CAST), target_type(target_type), operand(
				operand) {
}
ast::cast_expression::~cast_expression() {
	delete operand;
}
ast::type_ref ast::cast_expression::get_target_type() {
	return target_type;
}
void ast::cast_expression::set_target_type(ast::type_ref target_type) {
	this->target_type = target_type;
}
ast::expression* ast::cast_expression::get_operand() {
	return operand;
}
void ast::cast_expression::set_operand(ast::expression* operand) {
	this->operand = operand;
}
std::string ast::cast_expression::to_string() {
	return "cast_expr{(" + target_type.to_string() + ") " + operand->to_string()
			+ "}";
}
std::vector<ast::expression**>* ast::cast_expression::get_children() {
	return new std::vector<ast::expression**>(1, &operand);
}
void ast::cast_expression::accept(ast::ast_visitor* visitor) {
	visitor->visit_cast_expression(this);
}

ast::array_expression::array_expression(ast::expression* target,
		std::vector<ast::expression*>* indices) :
		expression(ast::expression_kind::ARRAY), target(target), indices(
				indices) {
}
ast::array_expression::~array_expression() {
	delete target;
	for (ast::expression*& index : *indices) {
		delete index;
	}
	delete indices;
}
ast::expression* ast::array_expression::get_target() {
	return target;
}
void ast::array_expression::set_target(ast::expression* target) {
	this->target = target;
}
std::vector<ast::expression*>* ast::array_expression::get_indices() {
	return indices;
}
std::string ast::array_expression::to_string() {
	std::string ret = "arr_expr{" + target->to_string() + "[";
	bool is_first = true;
	for (ast::expression*& index : *indices) {
		if (!is_first) {
			ret += ", ";
		}
		ret += index->to_string();
		is_first = false;
	}
	ret += "]}";
	return ret;
}
std::vector<ast::expression**>* ast::array_expression::get_children() {
	std::vector<ast::expression**>* children =
			new std::vector<ast::expression**>(indices->size() + 1);
	(*children)[0] = &target;
	int i = 1;
	for (auto& index : *indices) {
		(*children)[i++] = &index;
	}
	return children;
}
void ast::array_expression::accept(ast::ast_visitor* visitor) {
	visitor->visit_array_expression(this);
}

ast::statement::statement(ast::statement_kind kind) :
		kind(kind) {
}
ast::statement::~statement() {
}
ast::statement_kind ast::statement::get_statement_kind() {
	return kind;
}
bool ast::statement::is_of_statement_kind(ast::statement_kind kind) {
	return this->kind == kind;
}
ast::statement* ast::statement::get_parent_statement() {
	return parent_statement;
}
void ast::statement::set_parent_statement(ast::statement* parent) {
	parent_statement = parent;
}
ast::ast_node* ast::statement::get_parent_node() {
	return parent_node;
}
void ast::statement::set_parent_node(ast::ast_node* parent) {
	parent_node = parent;
}
std::string ast::statement::to_string() {
	std::cerr << "Called statement::to_string()!" << std::endl;
	throw std::exception();
}
std::vector<ast::statement**>* ast::statement::get_child_statements() {
	return new std::vector<ast::statement**>(0);
}
std::vector<ast::expression**>* ast::statement::get_child_expressions() {
	return new std::vector<ast::expression**>(0);
}
void ast::statement::accept(ast::ast_visitor* visitor) {
	std::cerr << "Called statement::accept(ast_visitor*)!" << std::endl;
	throw std::exception();
}

ast::block_statement::block_statement(std::vector<ast::statement*>* children) :
		statement(ast::statement_kind::BLOCK), children(children) {
}
ast::block_statement::~block_statement() {
	for (ast::statement*& child : *children) {
		delete child;
	}
	delete children;
}
std::vector<ast::statement*>* ast::block_statement::get_children() {
	return children;
}
std::string ast::block_statement::to_string() {
	std::string ret = "block_stmt{";
	for (ast::statement*& child : *children) {
		ret += "\n";
		ret += child->to_string();
	}
	ret += "\n";
	ret += "}";
	return ret;
}
std::vector<ast::statement**>* ast::block_statement::get_child_statements() {
	std::vector<ast::statement**>* ret = new std::vector<ast::statement**>;
	for (ast::statement*& child : *children) {
		ret->push_back(&child);
	}
	return ret;
}
void ast::block_statement::accept(ast::ast_visitor* visitor) {
	visitor->visit_block_statement(this);
}

ast::variable_declaration_statement::variable_declaration_statement(
		std::set<ast::modifier>* modifiers, ast::type_ref type,
		std::string name, ast::expression* initialization_expression) :
		statement(ast::statement_kind::VARIABLE_DECLARATION), modifiers(
				modifiers), type(type), name(name), initialization_expression(
				initialization_expression) {
}
ast::variable_declaration_statement::~variable_declaration_statement() {
	delete modifiers;
	delete initialization_expression;
}
std::set<ast::modifier>* ast::variable_declaration_statement::get_modifiers() {
	return modifiers;
}
ast::type_ref ast::variable_declaration_statement::get_type() {
	return type;
}
void ast::variable_declaration_statement::set_type(ast::type_ref type) {
	this->type = type;
}
std::string ast::variable_declaration_statement::get_name() {
	return name;
}
void ast::variable_declaration_statement::set_name(std::string name) {
	this->name = name;
}
ast::expression* ast::variable_declaration_statement::get_initialization_expression() {
	return initialization_expression;
}
void ast::variable_declaration_statement::set_initialization_expression(
		ast::expression* initialization_expression) {
	this->initialization_expression = initialization_expression;
}
std::string ast::variable_declaration_statement::to_string() {
	std::string ret = "vardecl_stmt{";
	bool is_first = true;
	for (ast::modifier mod : *modifiers) {
		if (!is_first) {
			ret += " ";
		}
		ret += static_cast<int>(mod);
		is_first = false;
	}
	ret += type.to_string() + " " + name;
	if (initialization_expression != nullptr) {
		ret += " = " + initialization_expression->to_string();
	}
	ret += "}";
	return ret;
}
std::vector<ast::expression**>* ast::variable_declaration_statement::get_child_expressions() {
	return new std::vector<ast::expression**>(1, &initialization_expression);
}
void ast::variable_declaration_statement::accept(ast::ast_visitor* visitor) {
	visitor->visit_variable_declaration_statement(this);
}

ast::assignment_statement::assignment_statement(ast::expression* lhs,
		std::string assignment_operator, ast::expression* rhs) :
		statement(ast::statement_kind::ASSIGNMENT), lhs(lhs), assignment_operator(
				assignment_operator), rhs(rhs) {
}
ast::assignment_statement::~assignment_statement() {
	delete lhs;
	delete rhs;
}
ast::expression* ast::assignment_statement::get_lhs() {
	return lhs;
}
void ast::assignment_statement::set_lhs(ast::expression* lhs) {
	this->lhs = lhs;
}
std::string ast::assignment_statement::get_assignment_operator() {
	return assignment_operator;
}
void ast::assignment_statement::set_assignment_operator(
		std::string assignment_operator) {
	this->assignment_operator = assignment_operator;
}
ast::expression* ast::assignment_statement::get_rhs() {
	return rhs;
}
void ast::assignment_statement::set_rhs(ast::expression* rhs) {
	this->rhs = rhs;
}
std::string ast::assignment_statement::to_string() {
	return "assign_stmt{" + lhs->to_string() + " " + assignment_operator + " "
			+ rhs->to_string() + "}";
}
std::vector<ast::expression**>* ast::assignment_statement::get_child_expressions() {
	std::vector<ast::expression**>* children =
			new std::vector<ast::expression**>(2);
	(*children)[0] = &lhs;
	(*children)[1] = &rhs;
	return children;
}
void ast::assignment_statement::accept(ast::ast_visitor* visitor) {
	visitor->visit_assignment_statement(this);
}

ast::if_statement::if_statement(ast::expression* condition,
		ast::statement* if_clause) :
		statement(ast::statement_kind::IF), condition(condition), if_clause(
				if_clause), else_clause(nullptr) {
}
ast::if_statement::if_statement(ast::expression* condition,
		ast::statement* if_clause, ast::statement* else_clause) :
		statement(ast::statement_kind::IF), condition(condition), if_clause(
				if_clause), else_clause(else_clause) {
}
ast::if_statement::~if_statement() {
	delete condition;
	delete if_clause;
	delete else_clause;
}
ast::expression* ast::if_statement::get_condition() {
	return condition;
}
void ast::if_statement::set_condition(ast::expression* condition) {
	this->condition = condition;
}
ast::statement* ast::if_statement::get_if_clause() {
	return if_clause;
}
void ast::if_statement::set_if_clause(ast::statement* if_clause) {
	this->if_clause = if_clause;
}
bool ast::if_statement::has_else_clause() {
	return else_clause != nullptr;
}
ast::statement* ast::if_statement::get_else_clause() {
	return else_clause;
}
void ast::if_statement::set_else_clause(ast::statement* else_clause) {
	this->else_clause = else_clause;
}
std::string ast::if_statement::to_string() {
	std::string ret = "if_stmt{" + condition->to_string();
	ret += "\n";
	ret += "then:";
	ret += "\n";
	ret += if_clause->to_string();
	if (has_else_clause()) {
		ret += "\n";
		ret += "else:";
		ret += "\n";
		ret += else_clause->to_string();
	}
	ret += "\n";
	ret += "}";
	return ret;
}
std::vector<ast::statement**>* ast::if_statement::get_child_statements() {
	std::vector<ast::statement**>* children = new std::vector<ast::statement**>(
			has_else_clause() ? 2 : 1);
	(*children)[0] = &if_clause;
	if (has_else_clause()) {
		(*children)[1] = &else_clause;
	}
	return children;
}
std::vector<ast::expression**>* ast::if_statement::get_child_expressions() {
	return new std::vector<ast::expression**>(1, &condition);
}
void ast::if_statement::accept(ast::ast_visitor* visitor) {
	visitor->visit_if_statement(this);
}

ast::while_statement::while_statement(ast::expression* condition,
		ast::statement* while_clause) :
		statement(ast::statement_kind::WHILE), condition(condition), while_clause(
				while_clause) {
}
ast::while_statement::~while_statement() {
	delete condition;
	delete while_clause;
}
ast::expression* ast::while_statement::get_condition() {
	return condition;
}
void ast::while_statement::set_condition(ast::expression* condition) {
	this->condition = condition;
}
ast::statement* ast::while_statement::get_while_clause() {
	return while_clause;
}
void ast::while_statement::set_while_clause(ast::statement* while_clause) {
	this->while_clause = while_clause;
}
std::string ast::while_statement::to_string() {
	std::string ret = "while_stmt{" + condition->to_string();
	ret += "\n";
	ret += while_clause->to_string();
	ret += "\n";
	ret += "}";
	return ret;
}
std::vector<ast::statement**>* ast::while_statement::get_child_statements() {
	return new std::vector<ast::statement**>(1, &while_clause);
}
std::vector<ast::expression**>* ast::while_statement::get_child_expressions() {
	return new std::vector<ast::expression**>(1, &condition);
}
void ast::while_statement::accept(ast::ast_visitor* visitor) {
	visitor->visit_while_statement(this);
}

ast::do_while_statement::do_while_statement(ast::statement* do_while_clause,
		ast::expression* condition) :
		statement(ast::statement_kind::DO_WHILE), do_while_clause(
				do_while_clause), condition(condition) {
}
ast::do_while_statement::~do_while_statement() {
	delete do_while_clause;
	delete condition;
}
ast::statement* ast::do_while_statement::get_do_while_clause() {
	return do_while_clause;
}
void ast::do_while_statement::set_do_while_clause(
		ast::statement* do_while_clause) {
	this->do_while_clause = do_while_clause;
}
ast::expression* ast::do_while_statement::get_condition() {
	return condition;
}
void ast::do_while_statement::set_condition(ast::expression* condition) {
	this->condition = condition;
}
std::string ast::do_while_statement::to_string() {
	std::string ret = "do_while_stmt{";
	ret += "\n";
	ret += do_while_clause->to_string();
	ret += "\n";
	ret += "while: ";
	ret += condition->to_string();
	ret += "\n";
	ret += "}";
	return ret;
}
std::vector<ast::statement**>* ast::do_while_statement::get_child_statements() {
	return new std::vector<ast::statement**>(1, &do_while_clause);
}
std::vector<ast::expression**>* ast::do_while_statement::get_child_expressions() {
	return new std::vector<ast::expression**>(1, &condition);
}
void ast::do_while_statement::accept(ast::ast_visitor* visitor) {
	visitor->visit_do_while_statement(this);
}

ast::repeat_statement::repeat_statement(ast::expression* times,
		ast::statement* repeat_clause) :
		statement(ast::statement_kind::REPEAT), times(times), repeat_clause(
				repeat_clause) {
}
ast::repeat_statement::~repeat_statement() {
	delete times;
	delete repeat_clause;
}
ast::expression* ast::repeat_statement::get_times() {
	return times;
}
void ast::repeat_statement::set_times(ast::expression* times) {
	this->times = times;
}
ast::statement* ast::repeat_statement::get_repeat_clause() {
	return repeat_clause;
}
void ast::repeat_statement::set_repeat_clause(ast::statement* repeat_clause) {
	this->repeat_clause = repeat_clause;
}
std::string ast::repeat_statement::to_string() {
	std::string ret = "repeat_stmt{times=" + times->to_string();
	ret += "\n";
	ret += repeat_clause->to_string();
	ret += "\n";
	ret += "}";
	return ret;
}
std::vector<ast::statement**>* ast::repeat_statement::get_child_statements() {
	return new std::vector<ast::statement**>(1, &repeat_clause);
}
std::vector<ast::expression**>* ast::repeat_statement::get_child_expressions() {
	return new std::vector<ast::expression**>(1, &times);
}
void ast::repeat_statement::accept(ast::ast_visitor* visitor) {
	visitor->visit_repeat_statement(this);
}

ast::for_statement::for_statement(ast::statement* initializer,
		ast::expression* condition, ast::statement* increment,
		ast::statement* for_clause) :
		statement(statement_kind::FOR), initializer(initializer), condition(
				condition), increment(increment), for_clause(for_clause) {

}
ast::for_statement::~for_statement() {
	delete initializer;
	delete condition;
	delete increment;
	delete for_clause;
}
bool ast::for_statement::has_initializer() {
	return initializer != nullptr;
}
ast::statement* ast::for_statement::get_initializer() {
	return initializer;
}
void ast::for_statement::set_initializer(ast::statement* initializer) {
	this->initializer = initializer;
}
bool ast::for_statement::has_condition() {
	return condition != nullptr;
}
ast::expression* ast::for_statement::get_condition() {
	return condition;
}
void ast::for_statement::set_condition(ast::expression* condition) {
	this->condition = condition;
}
bool ast::for_statement::has_increment() {
	return increment != nullptr;
}
ast::statement* ast::for_statement::get_increment() {
	return increment;
}
void ast::for_statement::set_increment(ast::statement* increment) {
	this->increment = increment;
}
ast::statement* ast::for_statement::get_for_clause() {
	return for_clause;
}
void ast::for_statement::set_for_clause(ast::statement* for_clause) {
	this->for_clause = for_clause;
}
std::string ast::for_statement::to_string() {
	std::string ret = "for_stmt{";
	ret += "\n";
	ret += "initializer: "
			+ (has_initializer() ? initializer->to_string() : "<none>");
	ret += "\n";
	ret += "condition: "
			+ (has_condition() ? condition->to_string() : "<none>");
	ret += "\n";
	ret += "increment: "
			+ (has_increment() ? increment->to_string() : "<none>");
	ret += "\n";
	ret += "do:";
	ret += "\n";
	ret += for_clause->to_string();
	ret += "\n";
	ret += "}";
	return ret;
}
std::vector<ast::statement**>* ast::for_statement::get_child_statements() {
	std::vector<ast::statement**>* children = new std::vector<ast::statement**>;
	if (has_initializer()) {
		children->push_back(&initializer);
	}
	if (has_increment()) {
		children->push_back(&increment);
	}
	children->push_back(&for_clause);
	return children;
}
std::vector<ast::expression**>* ast::for_statement::get_child_expressions() {
	if (has_condition()) {
		return new std::vector<ast::expression**>(1, &condition);
	} else {
		return new std::vector<ast::expression**>(0);
	}
}
void ast::for_statement::accept(ast::ast_visitor* visitor) {
	visitor->visit_for_statement(this);
}

ast::forever_statement::forever_statement(ast::statement* forever_clause) :
		statement(ast::statement_kind::FOREVER), forever_clause(forever_clause) {
}
ast::forever_statement::~forever_statement() {
	delete forever_clause;
}
ast::statement* ast::forever_statement::get_forever_clause() {
	return forever_clause;
}
void ast::forever_statement::set_forever_clause(
		ast::statement* forever_clause) {
	this->forever_clause = forever_clause;
}
std::string ast::forever_statement::to_string() {
	std::string ret = "forever_stmt{";
	ret += "\n";
	ret += forever_clause->to_string();
	ret += "\n";
	ret += "}";
	return ret;
}
std::vector<ast::statement**>* ast::forever_statement::get_child_statements() {
	return new std::vector<ast::statement**>(1, &forever_clause);
}
void ast::forever_statement::accept(ast::ast_visitor* visitor) {
	visitor->visit_forever_statement(this);
}

ast::return_statement::return_statement(ast::expression* operand) :
		statement(ast::statement_kind::RETURN), operand(operand) {
}
ast::return_statement::~return_statement() {
	delete operand;
}
ast::expression* ast::return_statement::get_operand() {
	return operand;
}
void ast::return_statement::set_operand(ast::expression* operand) {
	this->operand = operand;
}
std::string ast::return_statement::to_string() {
	return "ret_stmt{" + operand->to_string() + "}";
}
std::vector<ast::expression**>* ast::return_statement::get_child_expressions() {
	return new std::vector<ast::expression**>(1, &operand);
}
void ast::return_statement::accept(ast::ast_visitor* visitor) {
	visitor->visit_return_statement(this);
}

ast::expression_statement::expression_statement(ast::expression* expr) :
		statement(ast::statement_kind::EXPRESSION), expr(expr) {
}
ast::expression_statement::~expression_statement() {
	delete expr;
}
ast::expression* ast::expression_statement::get_expression() {
	return expr;
}
void ast::expression_statement::set_expression(ast::expression* expr) {
	this->expr = expr;
}
std::string ast::expression_statement::to_string() {
	return "expr_stmt{" + expr->to_string() + "}";
}
std::vector<ast::expression**>* ast::expression_statement::get_child_expressions() {
	return new std::vector<ast::expression**>(1, &expr);
}
void ast::expression_statement::accept(ast::ast_visitor* visitor) {
	visitor->visit_expression_statement(this);
}

ast::ast_node::ast_node(ast::ast_node_kind kind) :
		kind(kind) {
}
ast::ast_node::~ast_node() {
}
ast::ast_node_kind ast::ast_node::get_ast_node_kind() {
	return kind;
}
bool ast::ast_node::is_of_ast_node_kind(ast::ast_node_kind kind) {
	return this->kind == kind;
}
ast::ast_node* ast::ast_node::get_parent_node() {
	return parent_node;
}
void ast::ast_node::set_parent_node(ast::ast_node* parent) {
	parent_node = parent;
}
std::string ast::ast_node::to_string() {
	std::cerr << "Called ast_node::to_string()!" << std::endl;
	throw std::exception();
}
std::vector<ast::ast_node**>* ast::ast_node::get_child_nodes() {
	return new std::vector<ast::ast_node**>(0);
}
std::vector<ast::statement**>* ast::ast_node::get_child_statements() {
	return new std::vector<ast::statement**>(0);
}
std::vector<ast::expression**>* ast::ast_node::get_child_expressions() {
	return new std::vector<ast::expression**>(0);
}
void ast::ast_node::accept(ast::ast_visitor* visitor) {
	std::cerr << "Called ast_node::accept(ast_visitor*)!" << std::endl;
}

ast::module_node::module_node(std::string namespace_name,
		std::vector<ast::ast_node*>* children) :
		ast_node(ast::ast_node_kind::MODULE), namespace_name(namespace_name), children(
				children) {
}
ast::module_node::module_node(std::vector<ast::ast_node*>* children) :
		ast_node(ast::ast_node_kind::MODULE), namespace_name(""), children(
				children) {
}
ast::module_node::~module_node() {
	for (ast::ast_node*& child : *children) {
		delete child;
	}
	delete children;
}
std::string ast::module_node::get_namespace() {
	return namespace_name;
}
void ast::module_node::set_namespace(std::string namespace_name) {
	this->namespace_name = namespace_name;
}
std::vector<ast::ast_node*>* ast::module_node::get_children() {
	return children;
}
std::string ast::module_node::to_string() {
	std::string ret = "module_node{";
	if (!namespace_name.empty()) {
		ret += "ns=" + namespace_name;
	}
	for (ast::ast_node*& child : *children) {
		ret += "\n";
		ret += child->to_string();
	}
	ret += "\n";
	ret += "}";
	return ret;
}
std::vector<ast::ast_node**>* ast::module_node::get_child_nodes() {
	std::vector<ast::ast_node**>* ret = new std::vector<ast::ast_node**>;
	for (ast::ast_node*& child : *children) {
		ret->push_back(&child);
	}
	return ret;
}
void ast::module_node::accept(ast::ast_visitor* visitor) {
	visitor->visit_module_node(this);
}

ast::field_node::field_node(std::set<ast::modifier>* modifiers,
		ast::type_ref type, std::string name) :
		ast_node(ast::ast_node_kind::FIELD), modifiers(modifiers), type(type), name(
				name), initialization_expression(nullptr) {
}
ast::field_node::field_node(std::set<ast::modifier>* modifiers,
		ast::type_ref type, std::string name,
		ast::expression* initialization_expression) :
		ast_node(ast::ast_node_kind::FIELD), modifiers(modifiers), type(type), name(
				name), initialization_expression(initialization_expression) {
}
ast::field_node::~field_node() {
	delete modifiers;
	delete initialization_expression;
}
std::set<ast::modifier>* ast::field_node::get_modifiers() {
	return modifiers;
}
ast::type_ref ast::field_node::get_type() {
	return type;
}
void ast::field_node::set_type(ast::type_ref type) {
	this->type = type;
}
std::string ast::field_node::get_name() {
	return name;
}
void ast::field_node::set_name(std::string name) {
	this->name = name;
}
bool ast::field_node::has_initialization_expression() {
	return initialization_expression != nullptr;
}
ast::expression* ast::field_node::get_initialization_expression() {
	return initialization_expression;
}
void ast::field_node::set_initialization_expression(
		ast::expression* initialization_expression) {
	this->initialization_expression = initialization_expression;
}
std::string ast::field_node::to_string() {
	std::string ret = "field_node{";
	bool is_first = true;
	for (ast::modifier mod : *modifiers) {
		if (!is_first) {
			ret += " ";
		}
		ret += static_cast<int>(mod);
		is_first = false;
	}
	ret += " ";
	ret += type.to_string();
	ret += " ";
	ret += name;
	if (has_initialization_expression()) {
		ret += " = ";
		ret += initialization_expression->to_string();
	}
	ret += "}";
	return ret;
}
std::vector<ast::expression**>* ast::field_node::get_child_expressions() {
	if (has_initialization_expression()) {
		return new std::vector<ast::expression**>(1, &initialization_expression);
	} else {
		return new std::vector<ast::expression**>(0);
	}
}
void ast::field_node::accept(ast::ast_visitor* visitor) {
	visitor->visit_field_node(this);
}

ast::function_node::function_node(std::set<ast::modifier>* modifiers,
		ast::type_ref return_type, std::string name,
		std::vector<ast::field_node*>* parameters, ast::statement* body) :
		ast_node(ast::ast_node_kind::FUNCTION), modifiers(modifiers), return_type(
				return_type), name(name), parameters(parameters), body(body) {
}
ast::function_node::~function_node() {
	delete modifiers;
	for (ast::field_node*& param : *parameters) {
		delete param;
	}
	delete parameters;
	delete body;
}
std::set<ast::modifier>* ast::function_node::get_modifiers() {
	return modifiers;
}
ast::type_ref ast::function_node::get_return_type() {
	return return_type;
}
void ast::function_node::set_return_type(ast::type_ref return_type) {
	this->return_type = return_type;
}
std::string ast::function_node::get_name() {
	return name;
}
void ast::function_node::set_name(std::string name) {
	this->name = name;
}
std::vector<ast::field_node*>* ast::function_node::get_parameters() {
	return parameters;
}
ast::statement* ast::function_node::get_body() {
	return body;
}
void ast::function_node::set_body(ast::statement* body) {
	this->body = body;
}
std::string ast::function_node::to_string() {
	std::string ret = "func_node{";
	bool is_first = true;
	for (ast::modifier mod : *modifiers) {
		if (!is_first) {
			ret += " ";
		}
		ret += static_cast<int>(mod);
		is_first = false;
	}
	ret += " ";
	ret += return_type.to_string();
	ret += " ";
	ret += name;
	ret += "(";
	is_first = true;
	for (ast::field_node*& param : *parameters) {
		if (!is_first) {
			ret += ", ";
		}
		ret += param->to_string();
		is_first = false;
	}
	ret += ") ";
	ret += body->to_string();
	ret += "\n";
	ret += "}";
	return ret;
}
std::vector<ast::statement**>* ast::function_node::get_child_statements() {
	return new std::vector<ast::statement**>(1, &body);
}
std::vector<ast::expression**>* ast::function_node::get_child_expressions() {
	std::vector<ast::expression**>* children =
			new std::vector<ast::expression**>;
	for (const auto& param : *parameters) {
		if (param->has_initialization_expression()) {
			// need to do it this way to get the ptr to the original
			std::vector<ast::expression**>* param_children =
					param->get_child_expressions();
			children->push_back((*param_children)[0]);
			delete param_children;
		}
	}
	return children;
}
void ast::function_node::accept(ast::ast_visitor* visitor) {
	visitor->visit_function_node(this);
}

ast::ast_visitor::~ast_visitor() {
}
void ast::ast_visitor::visit_identifier_expression(
		ast::identifier_expression* expr) {
}
void ast::ast_visitor::visit_parenthesized_expression(
		ast::parenthesized_expression* expr) {
}
void ast::ast_visitor::visit_call_expression(ast::call_expression* expr) {
}
void ast::ast_visitor::visit_namespace_expression(
		ast::namespace_expression* expr) {
}
void ast::ast_visitor::visit_operator_expression(
		ast::operator_expression* expr) {
}
void ast::ast_visitor::visit_unary_operator_left_expression(
		ast::unary_operator_left_expression* expr) {
}
void ast::ast_visitor::visit_unary_operator_right_expression(
		ast::unary_operator_right_expression* expr) {
}
void ast::ast_visitor::visit_const_boolean_expression(
		ast::const_boolean_expression* expr) {
}
void ast::ast_visitor::visit_const_integer_expression(
		ast::const_integer_expression* expr) {
}
void ast::ast_visitor::visit_const_float_expression(
		ast::const_float_expression* expr) {
}
void ast::ast_visitor::visit_const_double_expression(
		ast::const_double_expression* expr) {
}
void ast::ast_visitor::visit_const_string_expression(
		ast::const_string_expression* expr) {
}
void ast::ast_visitor::visit_cast_expression(ast::cast_expression* expr) {
}
void ast::ast_visitor::visit_array_expression(ast::array_expression* expr) {
}
void ast::ast_visitor::visit_block_statement(ast::block_statement* stmt) {
}
void ast::ast_visitor::visit_variable_declaration_statement(
		ast::variable_declaration_statement* stmt) {
}
void ast::ast_visitor::visit_assignment_statement(
		ast::assignment_statement* stmt) {
}
void ast::ast_visitor::visit_if_statement(ast::if_statement* stmt) {
}
void ast::ast_visitor::visit_while_statement(ast::while_statement* stmt) {
}
void ast::ast_visitor::visit_do_while_statement(ast::do_while_statement* stmt) {
}
void ast::ast_visitor::visit_for_statement(ast::for_statement* stmt) {
}
void ast::ast_visitor::visit_forever_statement(ast::forever_statement* stmt) {
}
void ast::ast_visitor::visit_repeat_statement(ast::repeat_statement* stmt) {
}
void ast::ast_visitor::visit_return_statement(ast::return_statement* stmt) {
}
void ast::ast_visitor::visit_expression_statement(
		ast::expression_statement* stmt) {
}
void ast::ast_visitor::visit_module_node(ast::module_node* node) {
}
void ast::ast_visitor::visit_field_node(ast::field_node* node) {
}
void ast::ast_visitor::visit_function_node(ast::function_node* node) {
}
void ast::ast_visitor::visit_expression(ast::expression* expr) {
	expr->accept(this);
	std::vector<ast::expression**>* children = expr->get_children();
	for (const auto& child : *children) {
		visit_expression(*child);
	}
	delete children;
}
void ast::ast_visitor::visit_statement(ast::statement* stmt) {
	stmt->accept(this);
	std::vector<ast::expression**>* child_expressions =
			stmt->get_child_expressions();
	for (const auto& child : *child_expressions) {
		visit_expression(*child);
	}
	delete child_expressions;
	std::vector<ast::statement**>* child_statements =
			stmt->get_child_statements();
	for (const auto& child : *child_statements) {
		visit_statement(*child);
	}
	delete child_statements;
}
void ast::ast_visitor::visit_ast_node(ast::ast_node* node) {
	node->accept(this);
	std::vector<ast::expression**>* child_expressions =
			node->get_child_expressions();
	for (const auto& child : *child_expressions) {
		visit_expression(*child);
	}
	delete child_expressions;
	std::vector<ast::statement**>* child_statements =
			node->get_child_statements();
	for (const auto& child : *child_statements) {
		visit_statement(*child);
	}
	delete child_statements;
	std::vector<ast::ast_node**>* child_nodes = node->get_child_nodes();
	for (const auto& child : *child_nodes) {
		visit_ast_node(*child);
	}
	delete child_nodes;
}
void ast::ast_visitor::visit_all(std::vector<ast::ast_node*>* nodes) {
	for (ast::ast_node*& node : *nodes) {
		visit_ast_node(node);
	}
}
