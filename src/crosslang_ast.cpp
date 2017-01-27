/*
 *      Author: Earthcomputer
 */

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include "crosslang_ast.hpp"

type_ref::type_ref(std::string type_name) :
		namespaces(), type_name(type_name), generic_args() {
}

type_ref::type_ref(std::vector<std::string>* namespaces, std::string type_name) :
		namespaces(namespaces), type_name(type_name), generic_args() {
}
type_ref::type_ref(std::vector<std::string>* namespaces, std::string type_name,
		std::vector<type_ref>* generic_args) :
		namespaces(namespaces), type_name(type_name), generic_args(generic_args) {
}
std::vector<std::string>* type_ref::get_namespaces() {
	return namespaces;
}
std::string type_ref::get_type_name() {
	return type_name;
}
void type_ref::set_type_name(std::string type_name) {
	this->type_name = type_name;
}
std::vector<type_ref>* type_ref::get_generic_args() {
	return generic_args;
}
bool type_ref::is_bool() {
	return namespaces->empty() && generic_args->empty()
			&& (type_name == "bool" || type_name == "boolean");
}
bool type_ref::is_char() {
	return namespaces->empty() && generic_args->empty() && type_name == "char";
}
bool type_ref::is_double() {
	return namespaces->empty() && generic_args->empty() && type_name == "double";
}
bool type_ref::is_float() {
	return namespaces->empty() && generic_args->empty() && type_name == "float";
}
bool type_ref::is_int() {
	return namespaces->empty() && generic_args->empty() && type_name == "int";
}
bool type_ref::is_long() {
	return namespaces->empty() && generic_args->empty() && type_name == "long";
}
bool type_ref::is_short() {
	return namespaces->empty() && generic_args->empty() && type_name == "short";
}
std::string type_ref::to_string() {
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
bool type_ref::operator ==(type_ref other) {
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
bool type_ref::operator !=(type_ref other) {
	return !operator==(other);
}

expression::expression(expression_kind kind) :
		kind(kind) {
}
expression::~expression() {
}
expression_kind expression::get_expression_kind() {
	return kind;
}
bool expression::is_of_expression_kind(expression_kind kind) {
	return this->kind == kind;
}
expression* expression::get_parent_expression() {
	return parent_expression;
}
void expression::set_parent_expression(expression* parent) {
	parent_expression = parent;
}
statement* expression::get_parent_statement() {
	return parent_statement;
}
void expression::set_parent_statement(statement* parent) {
	parent_statement = parent;
}
ast_node* expression::get_parent_node() {
	return parent_node;
}
void expression::set_parent_node(ast_node* parent) {
	parent_node = parent;
}
std::string expression::to_string() {
	std::cerr << "Called expression::to_string()!" << std::endl;
	throw std::exception();
}
std::vector<expression**>* expression::get_children() {
	return new std::vector<expression**>(0);
}
void expression::accept(ast_visitor* visitor) {
	std::cerr << "Called expression::accept(ast_visitor*)!" << std::endl;
	throw std::exception();
}

identifier_expression::identifier_expression(std::string identifier) :
		expression(expression_kind::IDENTIFIER), identifier(identifier) {
}
std::string identifier_expression::get_identifier() {
	return identifier;
}
void identifier_expression::set_identifier(std::string identifier) {
	this->identifier = identifier;
}
std::string identifier_expression::to_string() {
	return "id_expr{" + identifier + "}";
}
void identifier_expression::accept(ast_visitor* visitor) {
	visitor->visit_identifier_expression(this);
}

parenthesized_expression::parenthesized_expression(expression* child) :
		expression(expression_kind::PARENTHESIZED), child(child) {
}
parenthesized_expression::~parenthesized_expression() {
	delete child;
}
expression* parenthesized_expression::get_child() {
	return child;
}
void parenthesized_expression::set_child(expression* child) {
	this->child = child;
}
std::string parenthesized_expression::to_string() {
	return "par_expr{" + child->to_string() + "}";
}
std::vector<expression**>* parenthesized_expression::get_children() {
	return new std::vector<expression**>(1, &child);
}
void parenthesized_expression::accept(ast_visitor* visitor) {
	visitor->visit_parenthesized_expression(this);
}

call_expression::call_expression(std::string name,
		std::vector<expression*>* operands) :
		expression(expression_kind::CALL), name(name), operands(operands) {
}
call_expression::~call_expression() {
	for (expression*& operand : *operands) {
		delete operand;
	}
	delete operands;
}
std::string call_expression::get_name() {
	return name;
}
void call_expression::set_name(std::string name) {
	this->name = name;
}
std::vector<expression*>* call_expression::get_operands() {
	return operands;
}
std::string call_expression::to_string() {
	std::string ret = "call_expr{" + name + " with: ";
	bool is_first = true;
	for (expression*& operand : *operands) {
		if (!is_first) {
			ret += ", ";
		}
		ret += operand->to_string();
		is_first = false;
	}
	ret += "}";
	return ret;
}
std::vector<expression**>* call_expression::get_children() {
	std::vector<expression**>* children = new std::vector<expression**>;
	for (expression*& operand : *operands) {
		children->push_back(&operand);
	}
	return children;
}
void call_expression::accept(ast_visitor* visitor) {
	visitor->visit_call_expression(this);
}

namespace_expression::namespace_expression(std::string namespace_name,
		expression* operand) :
		expression(expression_kind::NAMESPACE), namespace_name(namespace_name), operand(
				operand) {
}
namespace_expression::~namespace_expression() {
	delete operand;
}
std::string namespace_expression::get_namespace() {
	return namespace_name;
}
void namespace_expression::set_namespace(std::string namespace_name) {
	this->namespace_name = namespace_name;
}
expression* namespace_expression::get_operand() {
	return operand;
}
void namespace_expression::set_operand(expression* operand) {
	this->operand = operand;
}
std::string namespace_expression::to_string() {
	return "ns_expr{" + namespace_name + "::" + operand->to_string() + "}";
}
std::vector<expression**>* namespace_expression::get_children() {
	return new std::vector<expression**>(1, &operand);
}
void namespace_expression::accept(ast_visitor* visitor) {
	visitor->visit_namespace_expression(this);
}

operator_expression::operator_expression(expression* lhs,
		std::string operator_name, expression* rhs) :
		expression(expression_kind::OPERATOR), lhs(lhs), operator_name(
				operator_name), rhs(rhs) {
}
operator_expression::~operator_expression() {
	delete lhs;
	delete rhs;
}
expression* operator_expression::get_lhs() {
	return lhs;
}
void operator_expression::set_lhs(expression* lhs) {
	this->lhs = lhs;
}
std::string operator_expression::get_operator() {
	return operator_name;
}
void operator_expression::set_operator(std::string operator_name) {
	this->operator_name = operator_name;
}
expression* operator_expression::get_rhs() {
	return rhs;
}
void operator_expression::set_rhs(expression* rhs) {
	this->rhs = rhs;
}
std::string operator_expression::to_string() {
	return "op_expr{" + lhs->to_string() + " " + operator_name + " "
			+ rhs->to_string() + "}";
}
std::vector<expression**>* operator_expression::get_children() {
	std::vector<expression**>* children = new std::vector<expression**>(2);
	(*children)[0] = &lhs;
	(*children)[1] = &rhs;
	return children;
}
void operator_expression::accept(ast_visitor* visitor) {
	visitor->visit_operator_expression(this);
}

unary_operator_left_expression::unary_operator_left_expression(
		std::string operator_name, expression* operand) :
		expression(expression_kind::UNARY_OPERATOR_LEFT), operator_name(
				operator_name), operand(operand) {
}
unary_operator_left_expression::~unary_operator_left_expression() {
	delete operand;
}
std::string unary_operator_left_expression::get_operator() {
	return operator_name;
}
void unary_operator_left_expression::set_operator(std::string operator_name) {
	this->operator_name = operator_name;
}
expression* unary_operator_left_expression::get_operand() {
	return operand;
}
void unary_operator_left_expression::set_operand(expression* operand) {
	this->operand = operand;
}
std::string unary_operator_left_expression::to_string() {
	return "unlop_expr{" + operator_name + " " + operand->to_string() + "}";
}
std::vector<expression**>* unary_operator_left_expression::get_children() {
	return new std::vector<expression**>(1, &operand);
}
void unary_operator_left_expression::accept(ast_visitor* visitor) {
	visitor->visit_unary_operator_left_expression(this);
}

unary_operator_right_expression::unary_operator_right_expression(
		expression* operand, std::string operator_name) :
		expression(expression_kind::UNARY_OPERATOR_RIGHT), operand(operand), operator_name(
				operator_name) {
}
unary_operator_right_expression::~unary_operator_right_expression() {
	delete operand;
}
expression* unary_operator_right_expression::get_operand() {
	return operand;
}
void unary_operator_right_expression::set_operand(expression* operand) {
	this->operand = operand;
}
std::string unary_operator_right_expression::get_operator() {
	return operator_name;
}
void unary_operator_right_expression::set_operator(std::string operator_name) {
	this->operator_name = operator_name;
}
std::string unary_operator_right_expression::to_string() {
	return "unrop_expr{" + operand->to_string() + " " + operator_name + "}";
}
std::vector<expression**>* unary_operator_right_expression::get_children() {
	return new std::vector<expression**>(1, &operand);
}
void unary_operator_right_expression::accept(ast_visitor* visitor) {
	visitor->visit_unary_operator_right_expression(this);
}

const_boolean_expression::const_boolean_expression(bool value) :
		expression(expression_kind::CONST_BOOLEAN), value(value) {
}
bool const_boolean_expression::get_value() {
	return value;
}
void const_boolean_expression::set_value(bool value) {
	this->value = value;
}
std::string const_boolean_expression::to_string() {
	std::string ret = "bool_expr{";
	if (value) {
		ret += "true";
	} else {
		ret += "false";
	}
	ret += "}";
	return ret;
}
void const_boolean_expression::accept(ast_visitor* visitor) {
	visitor->visit_const_boolean_expression(this);
}

const_integer_expression::const_integer_expression(int value) :
		expression(expression_kind::CONST_INTEGER), value(value), rad(
				radix::DECIMAL) {
}
const_integer_expression::const_integer_expression(int value, radix rad) :
		expression(expression_kind::CONST_INTEGER), value(value), rad(rad) {
}
int const_integer_expression::get_value() {
	return value;
}
void const_integer_expression::set_value(int value) {
	this->value = value;
}
radix const_integer_expression::get_radix() {
	return rad;
}
std::string const_integer_expression::to_string() {
	std::ostringstream ret;
	ret << "int_expr{" << value << ", rad=";
	switch (rad) {
	case radix::BINARY:
		ret << "bin";
		break;
	case radix::OCTAL:
		ret << "oct";
		break;
	case radix::DECIMAL:
		ret << "dec";
		break;
	case radix::HEX:
		ret << "hex";
		break;
	}
	ret << "}";
	return ret.str();
}
void const_integer_expression::accept(ast_visitor* visitor) {
	visitor->visit_const_integer_expression(this);
}

const_float_expression::const_float_expression(float value) :
		expression(expression_kind::CONST_FLOAT), value(value) {
}
float const_float_expression::get_value() {
	return value;
}
void const_float_expression::set_value(float value) {
	this->value = value;
}
std::string const_float_expression::to_string() {
	std::ostringstream ret;
	ret << "float_expr{" << value << "}";
	return ret.str();
}
void const_float_expression::accept(ast_visitor* visitor) {
	visitor->visit_const_float_expression(this);
}

const_double_expression::const_double_expression(double value) :
		expression(expression_kind::CONST_DOUBLE), value(value) {
}
double const_double_expression::get_value() {
	return value;
}
void const_double_expression::set_value(double value) {
	this->value = value;
}
std::string const_double_expression::to_string() {
	std::ostringstream ret;
	ret << "double_expr{" << value << "}";
	return ret.str();
}
void const_double_expression::accept(ast_visitor* visitor) {
	visitor->visit_const_double_expression(this);
}

const_string_expression::const_string_expression(std::string value) :
		expression(expression_kind::CONST_STRING), value(value) {
}
std::string const_string_expression::get_value() {
	return value;
}
void const_string_expression::set_value(std::string value) {
	this->value = value;
}
std::string const_string_expression::to_string() {
	return "str_expr{" + value + "}";
}
void const_string_expression::accept(ast_visitor* visitor) {
	visitor->visit_const_string_expression(this);
}

cast_expression::cast_expression(type_ref target_type, expression* operand) :
		expression(expression_kind::CAST), target_type(target_type), operand(
				operand) {
}
cast_expression::~cast_expression() {
	delete operand;
}
type_ref cast_expression::get_target_type() {
	return target_type;
}
void cast_expression::set_target_type(type_ref target_type) {
	this->target_type = target_type;
}
expression* cast_expression::get_operand() {
	return operand;
}
void cast_expression::set_operand(expression* operand) {
	this->operand = operand;
}
std::string cast_expression::to_string() {
	return "cast_expr{(" + target_type.to_string() + ") " + operand->to_string()
			+ "}";
}
std::vector<expression**>* cast_expression::get_children() {
	return new std::vector<expression**>(1, &operand);
}
void cast_expression::accept(ast_visitor* visitor) {
	visitor->visit_cast_expression(this);
}

array_expression::array_expression(expression* target,
		std::vector<expression*>* indices) :
		expression(expression_kind::ARRAY), target(target), indices(indices) {
}
array_expression::~array_expression() {
	delete target;
	for (expression*& index : *indices) {
		delete index;
	}
	delete indices;
}
expression* array_expression::get_target() {
	return target;
}
void array_expression::set_target(expression* target) {
	this->target = target;
}
std::vector<expression*>* array_expression::get_indices() {
	return indices;
}
std::string array_expression::to_string() {
	std::string ret = "arr_expr{" + target->to_string() + "[";
	bool is_first = true;
	for (expression*& index : *indices) {
		if (!is_first) {
			ret += ", ";
		}
		ret += index->to_string();
		is_first = false;
	}
	ret += "]}";
	return ret;
}
std::vector<expression**>* array_expression::get_children() {
	std::vector<expression**>* children = new std::vector<expression**>(
			indices->size() + 1);
	(*children)[0] = &target;
	int i = 1;
	for (auto& index : *indices) {
		(*children)[i++] = &index;
	}
	return children;
}
void array_expression::accept(ast_visitor* visitor) {
	visitor->visit_array_expression(this);
}

statement::statement(statement_kind kind) :
		kind(kind) {
}
statement::~statement() {
}
statement_kind statement::get_statement_kind() {
	return kind;
}
bool statement::is_of_statement_kind(statement_kind kind) {
	return this->kind == kind;
}
statement* statement::get_parent_statement() {
	return parent_statement;
}
void statement::set_parent_statement(statement* parent) {
	parent_statement = parent;
}
ast_node* statement::get_parent_node() {
	return parent_node;
}
void statement::set_parent_node(ast_node* parent) {
	parent_node = parent;
}
std::string statement::to_string() {
	std::cerr << "Called statement::to_string()!" << std::endl;
	throw std::exception();
}
std::vector<statement**>* statement::get_child_statements() {
	return new std::vector<statement**>(0);
}
std::vector<expression**>* statement::get_child_expressions() {
	return new std::vector<expression**>(0);
}
void statement::accept(ast_visitor* visitor) {
	std::cerr << "Called statement::accept(ast_visitor*)!" << std::endl;
	throw std::exception();
}

block_statement::block_statement(std::vector<statement*>* children) :
		statement(statement_kind::BLOCK), children(children) {
}
block_statement::~block_statement() {
	for (statement*& child : *children) {
		delete child;
	}
	delete children;
}
std::vector<statement*>* block_statement::get_children() {
	return children;
}
std::string block_statement::to_string() {
	std::string ret = "block_stmt{";
	for (statement*& child : *children) {
		ret += "\n";
		ret += child->to_string();
	}
	ret += "\n";
	ret += "}";
	return ret;
}
std::vector<statement**>* block_statement::get_child_statements() {
	std::vector<statement**>* ret = new std::vector<statement**>;
	for (statement*& child : *children) {
		ret->push_back(&child);
	}
	return ret;
}
void block_statement::accept(ast_visitor* visitor) {
	visitor->visit_block_statement(this);
}

variable_declaration_statement::variable_declaration_statement(
		std::set<modifier>* modifiers, type_ref type, std::string name,
		expression* initialization_expression) :
		statement(statement_kind::VARIABLE_DECLARATION), modifiers(modifiers), type(
				type), name(name), initialization_expression(
				initialization_expression) {
}
variable_declaration_statement::~variable_declaration_statement() {
	delete modifiers;
	delete initialization_expression;
}
std::set<modifier>* variable_declaration_statement::get_modifiers() {
	return modifiers;
}
type_ref variable_declaration_statement::get_type() {
	return type;
}
void variable_declaration_statement::set_type(type_ref type) {
	this->type = type;
}
std::string variable_declaration_statement::get_name() {
	return name;
}
void variable_declaration_statement::set_name(std::string name) {
	this->name = name;
}
expression* variable_declaration_statement::get_initialization_expression() {
	return initialization_expression;
}
void variable_declaration_statement::set_initialization_expression(
		expression* initialization_expression) {
	this->initialization_expression = initialization_expression;
}
std::string variable_declaration_statement::to_string() {
	std::string ret = "vardecl_stmt{";
	bool is_first = true;
	for (modifier mod : *modifiers) {
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
std::vector<expression**>* variable_declaration_statement::get_child_expressions() {
	return new std::vector<expression**>(1, &initialization_expression);
}
void variable_declaration_statement::accept(ast_visitor* visitor) {
	visitor->visit_variable_declaration_statement(this);
}

assignment_statement::assignment_statement(expression* lhs,
		std::string assignment_operator, expression* rhs) :
		statement(statement_kind::ASSIGNMENT), lhs(lhs), assignment_operator(
				assignment_operator), rhs(rhs) {
}
assignment_statement::~assignment_statement() {
	delete lhs;
	delete rhs;
}
expression* assignment_statement::get_lhs() {
	return lhs;
}
void assignment_statement::set_lhs(expression* lhs) {
	this->lhs = lhs;
}
std::string assignment_statement::get_assignment_operator() {
	return assignment_operator;
}
void assignment_statement::set_assignment_operator(
		std::string assignment_operator) {
	this->assignment_operator = assignment_operator;
}
expression* assignment_statement::get_rhs() {
	return rhs;
}
void assignment_statement::set_rhs(expression* rhs) {
	this->rhs = rhs;
}
std::string assignment_statement::to_string() {
	return "assign_stmt{" + lhs->to_string() + " " + assignment_operator + " "
			+ rhs->to_string() + "}";
}
std::vector<expression**>* assignment_statement::get_child_expressions() {
	std::vector<expression**>* children = new std::vector<expression**>(2);
	(*children)[0] = &lhs;
	(*children)[1] = &rhs;
	return children;
}
void assignment_statement::accept(ast_visitor* visitor) {
	visitor->visit_assignment_statement(this);
}

if_statement::if_statement(expression* condition, statement* if_clause) :
		statement(statement_kind::IF), condition(condition), if_clause(
				if_clause), else_clause(nullptr) {
}
if_statement::if_statement(expression* condition, statement* if_clause,
		statement* else_clause) :
		statement(statement_kind::IF), condition(condition), if_clause(
				if_clause), else_clause(else_clause) {
}
if_statement::~if_statement() {
	delete condition;
	delete if_clause;
	delete else_clause;
}
expression* if_statement::get_condition() {
	return condition;
}
void if_statement::set_condition(expression* condition) {
	this->condition = condition;
}
statement* if_statement::get_if_clause() {
	return if_clause;
}
void if_statement::set_if_clause(statement* if_clause) {
	this->if_clause = if_clause;
}
bool if_statement::has_else_clause() {
	return else_clause != nullptr;
}
statement* if_statement::get_else_clause() {
	return else_clause;
}
void if_statement::set_else_clause(statement* else_clause) {
	this->else_clause = else_clause;
}
std::string if_statement::to_string() {
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
std::vector<statement**>* if_statement::get_child_statements() {
	std::vector<statement**>* children = new std::vector<statement**>(
			has_else_clause() ? 2 : 1);
	(*children)[0] = &if_clause;
	if (has_else_clause()) {
		(*children)[1] = &else_clause;
	}
	return children;
}
std::vector<expression**>* if_statement::get_child_expressions() {
	return new std::vector<expression**>(1, &condition);
}
void if_statement::accept(ast_visitor* visitor) {
	visitor->visit_if_statement(this);
}

while_statement::while_statement(expression* condition, statement* while_clause) :
		statement(statement_kind::WHILE), condition(condition), while_clause(
				while_clause) {
}
while_statement::~while_statement() {
	delete condition;
	delete while_clause;
}
expression* while_statement::get_condition() {
	return condition;
}
void while_statement::set_condition(expression* condition) {
	this->condition = condition;
}
statement* while_statement::get_while_clause() {
	return while_clause;
}
void while_statement::set_while_clause(statement* while_clause) {
	this->while_clause = while_clause;
}
std::string while_statement::to_string() {
	std::string ret = "while_stmt{" + condition->to_string();
	ret += "\n";
	ret += while_clause->to_string();
	ret += "\n";
	ret += "}";
	return ret;
}
std::vector<statement**>* while_statement::get_child_statements() {
	return new std::vector<statement**>(1, &while_clause);
}
std::vector<expression**>* while_statement::get_child_expressions() {
	return new std::vector<expression**>(1, &condition);
}
void while_statement::accept(ast_visitor* visitor) {
	visitor->visit_while_statement(this);
}

do_while_statement::do_while_statement(statement* do_while_clause,
		expression* condition) :
		statement(statement_kind::DO_WHILE), do_while_clause(do_while_clause), condition(
				condition) {
}
do_while_statement::~do_while_statement() {
	delete do_while_clause;
	delete condition;
}
statement* do_while_statement::get_do_while_clause() {
	return do_while_clause;
}
void do_while_statement::set_do_while_clause(statement* do_while_clause) {
	this->do_while_clause = do_while_clause;
}
expression* do_while_statement::get_condition() {
	return condition;
}
void do_while_statement::set_condition(expression* condition) {
	this->condition = condition;
}
std::string do_while_statement::to_string() {
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
std::vector<statement**>* do_while_statement::get_child_statements() {
	return new std::vector<statement**>(1, &do_while_clause);
}
std::vector<expression**>* do_while_statement::get_child_expressions() {
	return new std::vector<expression**>(1, &condition);
}
void do_while_statement::accept(ast_visitor* visitor) {
	visitor->visit_do_while_statement(this);
}

repeat_statement::repeat_statement(expression* times, statement* repeat_clause) :
		statement(statement_kind::REPEAT), times(times), repeat_clause(
				repeat_clause) {
}
repeat_statement::~repeat_statement() {
	delete times;
	delete repeat_clause;
}
expression* repeat_statement::get_times() {
	return times;
}
void repeat_statement::set_times(expression* times) {
	this->times = times;
}
statement* repeat_statement::get_repeat_clause() {
	return repeat_clause;
}
void repeat_statement::set_repeat_clause(statement* repeat_clause) {
	this->repeat_clause = repeat_clause;
}
std::string repeat_statement::to_string() {
	std::string ret = "repeat_stmt{times=" + times->to_string();
	ret += "\n";
	ret += repeat_clause->to_string();
	ret += "\n";
	ret += "}";
	return ret;
}
std::vector<statement**>* repeat_statement::get_child_statements() {
	return new std::vector<statement**>(1, &repeat_clause);
}
std::vector<expression**>* repeat_statement::get_child_expressions() {
	return new std::vector<expression**>(1, &times);
}
void repeat_statement::accept(ast_visitor* visitor) {
	visitor->visit_repeat_statement(this);
}

for_statement::for_statement(statement* initializer, expression* condition,
		statement* increment, statement* for_clause) :
		statement(statement_kind::FOR), initializer(initializer), condition(
				condition), increment(increment), for_clause(for_clause) {

}
for_statement::~for_statement() {
	delete initializer;
	delete condition;
	delete increment;
	delete for_clause;
}
bool for_statement::has_initializer() {
	return initializer != nullptr;
}
statement* for_statement::get_initializer() {
	return initializer;
}
void for_statement::set_initializer(statement* initializer) {
	this->initializer = initializer;
}
bool for_statement::has_condition() {
	return condition != nullptr;
}
expression* for_statement::get_condition() {
	return condition;
}
void for_statement::set_condition(expression* condition) {
	this->condition = condition;
}
bool for_statement::has_increment() {
	return increment != nullptr;
}
statement* for_statement::get_increment() {
	return increment;
}
void for_statement::set_increment(statement* increment) {
	this->increment = increment;
}
statement* for_statement::get_for_clause() {
	return for_clause;
}
void for_statement::set_for_clause(statement* for_clause) {
	this->for_clause = for_clause;
}
std::string for_statement::to_string() {
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
std::vector<statement**>* for_statement::get_child_statements() {
	std::vector<statement**>* children = new std::vector<statement**>;
	if (has_initializer()) {
		children->push_back(&initializer);
	}
	if (has_increment()) {
		children->push_back(&increment);
	}
	children->push_back(&for_clause);
	return children;
}
std::vector<expression**>* for_statement::get_child_expressions() {
	if (has_condition()) {
		return new std::vector<expression**>(1, &condition);
	} else {
		return new std::vector<expression**>(0);
	}
}
void for_statement::accept(ast_visitor* visitor) {
	visitor->visit_for_statement(this);
}

forever_statement::forever_statement(statement* forever_clause) :
		statement(statement_kind::FOREVER), forever_clause(forever_clause) {
}
forever_statement::~forever_statement() {
	delete forever_clause;
}
statement* forever_statement::get_forever_clause() {
	return forever_clause;
}
void forever_statement::set_forever_clause(statement* forever_clause) {
	this->forever_clause = forever_clause;
}
std::string forever_statement::to_string() {
	std::string ret = "forever_stmt{";
	ret += "\n";
	ret += forever_clause->to_string();
	ret += "\n";
	ret += "}";
	return ret;
}
std::vector<statement**>* forever_statement::get_child_statements() {
	return new std::vector<statement**>(1, &forever_clause);
}
void forever_statement::accept(ast_visitor* visitor) {
	visitor->visit_forever_statement(this);
}

return_statement::return_statement(expression* operand) :
		statement(statement_kind::RETURN), operand(operand) {
}
return_statement::~return_statement() {
	delete operand;
}
expression* return_statement::get_operand() {
	return operand;
}
void return_statement::set_operand(expression* operand) {
	this->operand = operand;
}
std::string return_statement::to_string() {
	return "ret_stmt{" + operand->to_string() + "}";
}
std::vector<expression**>* return_statement::get_child_expressions() {
	return new std::vector<expression**>(1, &operand);
}
void return_statement::accept(ast_visitor* visitor) {
	visitor->visit_return_statement(this);
}

expression_statement::expression_statement(expression* expr) :
		statement(statement_kind::EXPRESSION), expr(expr) {
}
expression_statement::~expression_statement() {
	delete expr;
}
expression* expression_statement::get_expression() {
	return expr;
}
void expression_statement::set_expression(expression* expr) {
	this->expr = expr;
}
std::string expression_statement::to_string() {
	return "expr_stmt{" + expr->to_string() + "}";
}
std::vector<expression**>* expression_statement::get_child_expressions() {
	return new std::vector<expression**>(1, &expr);
}
void expression_statement::accept(ast_visitor* visitor) {
	visitor->visit_expression_statement(this);
}

ast_node::ast_node(ast_node_kind kind) :
		kind(kind) {
}
ast_node::~ast_node() {
}
ast_node_kind ast_node::get_ast_node_kind() {
	return kind;
}
bool ast_node::is_of_ast_node_kind(ast_node_kind kind) {
	return this->kind == kind;
}
ast_node* ast_node::get_parent_node() {
	return parent_node;
}
void ast_node::set_parent_node(ast_node* parent) {
	parent_node = parent;
}
std::string ast_node::to_string() {
	std::cerr << "Called ast_node::to_string()!" << std::endl;
	throw std::exception();
}
std::vector<ast_node**>* ast_node::get_child_nodes() {
	return new std::vector<ast_node**>(0);
}
std::vector<statement**>* ast_node::get_child_statements() {
	return new std::vector<statement**>(0);
}
std::vector<expression**>* ast_node::get_child_expressions() {
	return new std::vector<expression**>(0);
}
void ast_node::accept(ast_visitor* visitor) {
	std::cerr << "Called ast_node::accept(ast_visitor*)!" << std::endl;
}

module_node::module_node(std::string namespace_name,
		std::vector<ast_node*>* children) :
		ast_node(ast_node_kind::MODULE), namespace_name(namespace_name), children(
				children) {
}
module_node::module_node(std::vector<ast_node*>* children) :
		ast_node(ast_node_kind::MODULE), namespace_name(""), children(children) {
}
module_node::~module_node() {
	for (ast_node*& child : *children) {
		delete child;
	}
	delete children;
}
std::string module_node::get_namespace() {
	return namespace_name;
}
void module_node::set_namespace(std::string namespace_name) {
	this->namespace_name = namespace_name;
}
std::vector<ast_node*>* module_node::get_children() {
	return children;
}
std::string module_node::to_string() {
	std::string ret = "module_node{";
	if (!namespace_name.empty()) {
		ret += "ns=" + namespace_name;
	}
	for (ast_node*& child : *children) {
		ret += "\n";
		ret += child->to_string();
	}
	ret += "\n";
	ret += "}";
	return ret;
}
std::vector<ast_node**>* module_node::get_child_nodes() {
	std::vector<ast_node**>* ret = new std::vector<ast_node**>;
	for (ast_node*& child : *children) {
		ret->push_back(&child);
	}
	return ret;
}
void module_node::accept(ast_visitor* visitor) {
	visitor->visit_module_node(this);
}

field_node::field_node(std::set<modifier>* modifiers, type_ref type,
		std::string name) :
		ast_node(ast_node_kind::FIELD), modifiers(modifiers), type(type), name(
				name), initialization_expression(nullptr) {
}
field_node::field_node(std::set<modifier>* modifiers, type_ref type,
		std::string name, expression* initialization_expression) :
		ast_node(ast_node_kind::FIELD), modifiers(modifiers), type(type), name(
				name), initialization_expression(initialization_expression) {
}
field_node::~field_node() {
	delete modifiers;
	delete initialization_expression;
}
std::set<modifier>* field_node::get_modifiers() {
	return modifiers;
}
type_ref field_node::get_type() {
	return type;
}
void field_node::set_type(type_ref type) {
	this->type = type;
}
std::string field_node::get_name() {
	return name;
}
void field_node::set_name(std::string name) {
	this->name = name;
}
bool field_node::has_initialization_expression() {
	return initialization_expression != nullptr;
}
expression* field_node::get_initialization_expression() {
	return initialization_expression;
}
void field_node::set_initialization_expression(
		expression* initialization_expression) {
	this->initialization_expression = initialization_expression;
}
std::string field_node::to_string() {
	std::string ret = "field_node{";
	bool is_first = true;
	for (modifier mod : *modifiers) {
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
std::vector<expression**>* field_node::get_child_expressions() {
	if (has_initialization_expression()) {
		return new std::vector<expression**>(1, &initialization_expression);
	} else {
		return new std::vector<expression**>(0);
	}
}
void field_node::accept(ast_visitor* visitor) {
	visitor->visit_field_node(this);
}

function_node::function_node(std::set<modifier>* modifiers,
		type_ref return_type, std::string name,
		std::vector<field_node*>* parameters, statement* body) :
		ast_node(ast_node_kind::FUNCTION), modifiers(modifiers), return_type(
				return_type), name(name), parameters(parameters), body(body) {
}
function_node::~function_node() {
	delete modifiers;
	for (field_node*& param : *parameters) {
		delete param;
	}
	delete parameters;
	delete body;
}
std::set<modifier>* function_node::get_modifiers() {
	return modifiers;
}
type_ref function_node::get_return_type() {
	return return_type;
}
void function_node::set_return_type(type_ref return_type) {
	this->return_type = return_type;
}
std::string function_node::get_name() {
	return name;
}
void function_node::set_name(std::string name) {
	this->name = name;
}
std::vector<field_node*>* function_node::get_parameters() {
	return parameters;
}
statement* function_node::get_body() {
	return body;
}
void function_node::set_body(statement* body) {
	this->body = body;
}
std::string function_node::to_string() {
	std::string ret = "func_node{";
	bool is_first = true;
	for (modifier mod : *modifiers) {
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
	for (field_node*& param : *parameters) {
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
std::vector<statement**>* function_node::get_child_statements() {
	return new std::vector<statement**>(1, &body);
}
std::vector<expression**>* function_node::get_child_expressions() {
	std::vector<expression**>* children = new std::vector<expression**>;
	for (const auto& param : *parameters) {
		if (param->has_initialization_expression()) {
			// need to do it this way to get the ptr to the original
			std::vector<expression**>* param_children =
					param->get_child_expressions();
			children->push_back((*param_children)[0]);
			delete param_children;
		}
	}
	return children;
}
void function_node::accept(ast_visitor* visitor) {
	visitor->visit_function_node(this);
}

ast_visitor::~ast_visitor() {
}
void ast_visitor::visit_identifier_expression(identifier_expression* expr) {
}
void ast_visitor::visit_parenthesized_expression(
		parenthesized_expression* expr) {
}
void ast_visitor::visit_call_expression(call_expression* expr) {
}
void ast_visitor::visit_namespace_expression(namespace_expression* expr) {
}
void ast_visitor::visit_operator_expression(operator_expression* expr) {
}
void ast_visitor::visit_unary_operator_left_expression(
		unary_operator_left_expression* expr) {
}
void ast_visitor::visit_unary_operator_right_expression(
		unary_operator_right_expression* expr) {
}
void ast_visitor::visit_const_boolean_expression(
		const_boolean_expression* expr) {
}
void ast_visitor::visit_const_integer_expression(
		const_integer_expression* expr) {
}
void ast_visitor::visit_const_float_expression(const_float_expression* expr) {
}
void ast_visitor::visit_const_double_expression(const_double_expression* expr) {
}
void ast_visitor::visit_const_string_expression(const_string_expression* expr) {
}
void ast_visitor::visit_cast_expression(cast_expression* expr) {
}
void ast_visitor::visit_array_expression(array_expression* expr) {
}
void ast_visitor::visit_block_statement(block_statement* stmt) {
}
void ast_visitor::visit_variable_declaration_statement(
		variable_declaration_statement* stmt) {
}
void ast_visitor::visit_assignment_statement(assignment_statement* stmt) {
}
void ast_visitor::visit_if_statement(if_statement* stmt) {
}
void ast_visitor::visit_while_statement(while_statement* stmt) {
}
void ast_visitor::visit_do_while_statement(do_while_statement* stmt) {
}
void ast_visitor::visit_for_statement(for_statement* stmt) {
}
void ast_visitor::visit_forever_statement(forever_statement* stmt) {
}
void ast_visitor::visit_repeat_statement(repeat_statement* stmt) {
}
void ast_visitor::visit_return_statement(return_statement* stmt) {
}
void ast_visitor::visit_expression_statement(expression_statement* stmt) {
}
void ast_visitor::visit_module_node(module_node* node) {
}
void ast_visitor::visit_field_node(field_node* node) {
}
void ast_visitor::visit_function_node(function_node* node) {
}
void ast_visitor::visit_expression(expression* expr) {
	expr->accept(this);
	std::vector<expression**>* children = expr->get_children();
	for (const auto& child : *children) {
		visit_expression(*child);
	}
	delete children;
}
void ast_visitor::visit_statement(statement* stmt) {
	stmt->accept(this);
	std::vector<expression**>* child_expressions =
			stmt->get_child_expressions();
	for (const auto& child : *child_expressions) {
		visit_expression(*child);
	}
	delete child_expressions;
	std::vector<statement**>* child_statements = stmt->get_child_statements();
	for (const auto& child : *child_statements) {
		visit_statement(*child);
	}
	delete child_statements;
}
void ast_visitor::visit_ast_node(ast_node* node) {
	node->accept(this);
	std::vector<expression**>* child_expressions =
			node->get_child_expressions();
	for (const auto& child : *child_expressions) {
		visit_expression(*child);
	}
	delete child_expressions;
	std::vector<statement**>* child_statements = node->get_child_statements();
	for (const auto& child : *child_statements) {
		visit_statement(*child);
	}
	delete child_statements;
	std::vector<ast_node**>* child_nodes = node->get_child_nodes();
	for (const auto& child : *child_nodes) {
		visit_ast_node(*child);
	}
	delete child_nodes;
}
void ast_visitor::visit_all(std::vector<ast_node*>* nodes) {
	for (ast_node*& node : *nodes) {
		visit_ast_node(node);
	}
}
