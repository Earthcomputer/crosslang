/*
 *      Author: Earthcomputer
 */

#ifndef RELEASE_CROSSLANG_AST_HPP_
#define RELEASE_CROSSLANG_AST_HPP_

#include <string>
#include <set>
#include <vector>

enum class expression_kind {
	IDENTIFIER,
	PARENTHESIZED,
	CALL,
	NAMESPACE,
	OPERATOR,
	UNARY_OPERATOR_LEFT,
	UNARY_OPERATOR_RIGHT,
	CONST_BOOLEAN,
	CONST_INTEGER,
	CONST_FLOAT,
	CONST_DOUBLE,
	CONST_STRING,
	CAST,
	ARRAY
};

enum class statement_kind {
	BLOCK,
	VARIABLE_DECLARATION,
	ASSIGNMENT,
	IF,
	WHILE,
	DO_WHILE,
	FOR,
	FOREVER,
	REPEAT,
	RETURN,
	EXPRESSION
};

enum class ast_node_kind {
	MODULE, FIELD, FUNCTION
};

enum class modifier {
	GLOBAL
};

enum class radix {
	BINARY, OCTAL, DECIMAL, HEX
};

class type_ref {
	std::vector<std::string>* namespaces;
	std::string type_name;
	std::vector<type_ref>* generic_args;
public:
	type_ref(std::string type_name);
	type_ref(std::vector<std::string>* namespaces, std::string type_name);
	type_ref(std::vector<std::string>* namespaces, std::string type_name,
			std::vector<type_ref>* generic_args);
	std::vector<std::string>* get_namespaces();
	std::string get_type_name();
	void set_type_name(std::string type_name);
	std::vector<type_ref>* get_generic_args();
	bool is_bool();
	bool is_char();
	bool is_double();
	bool is_float();
	bool is_int();
	bool is_long();
	bool is_short();
	std::string to_string();
	bool operator==(type_ref other);
	bool operator!=(type_ref other);
};

class expression;
class statement;
class ast_node;
class ast_visitor;

class expression {
	expression_kind kind;
	expression* parent_expression = nullptr;
	statement* parent_statement = nullptr;
	ast_node* parent_node = nullptr;
protected:
	expression(expression_kind kind);
public:
	virtual ~expression();
	expression_kind get_expression_kind();
	bool is_of_expression_kind(expression_kind kind);
	expression* get_parent_expression();
	void set_parent_expression(expression* parent);
	statement* get_parent_statement();
	void set_parent_statement(statement* parent);
	ast_node* get_parent_node();
	void set_parent_node(ast_node* parent);
	virtual std::string to_string();
	virtual std::vector<expression**>* get_children();
	virtual void accept(ast_visitor* visitor);
};

class identifier_expression: public expression {
	std::string identifier;
public:
	identifier_expression(std::string identifier);
	std::string get_identifier();
	void set_identifier(std::string identifier);
	std::string to_string();
	void accept(ast_visitor* visitor);
};

class parenthesized_expression: public expression {
	expression* child;
public:
	parenthesized_expression(expression* child);
	~parenthesized_expression();
	expression* get_child();
	void set_child(expression* child);
	std::string to_string();
	std::vector<expression**>* get_children();
	void accept(ast_visitor* visitor);
};

class call_expression: public expression {
	std::string name;
	std::vector<expression*>* operands;
public:
	call_expression(std::string name, std::vector<expression*>* operands);
	~call_expression();
	std::string get_name();
	void set_name(std::string name);
	std::vector<expression*>* get_operands();
	std::string to_string();
	std::vector<expression**>* get_children();
	void accept(ast_visitor* visitor);
};

class namespace_expression: public expression {
	std::string namespace_name;
	expression* operand;
public:
	namespace_expression(std::string namespace_name, expression* operand);
	~namespace_expression();
	std::string get_namespace();
	void set_namespace(std::string namepsace_name);
	expression* get_operand();
	void set_operand(expression* operand);
	std::string to_string();
	std::vector<expression**>* get_children();
	void accept(ast_visitor* visitor);
};

class operator_expression: public expression {
	expression* lhs;
	std::string operator_name;
	expression* rhs;
public:
	operator_expression(expression* lhs, std::string operator_name,
			expression* rhs);
	~operator_expression();
	expression* get_lhs();
	void set_lhs(expression* lhs);
	std::string get_operator();
	void set_operator(std::string operator_name);
	expression* get_rhs();
	void set_rhs(expression* rhs);
	std::string to_string();
	std::vector<expression**>* get_children();
	void accept(ast_visitor* visitor);
};

class unary_operator_left_expression: public expression {
	std::string operator_name;
	expression* operand;
public:
	unary_operator_left_expression(std::string operator_name,
			expression* operand);
	~unary_operator_left_expression();
	std::string get_operator();
	void set_operator(std::string operator_name);
	expression* get_operand();
	void set_operand(expression* operand);
	std::string to_string();
	std::vector<expression**>* get_children();
	void accept(ast_visitor* visitor);
};

class unary_operator_right_expression: public expression {
	expression* operand;
	std::string operator_name;
public:
	unary_operator_right_expression(expression* operand,
			std::string operator_name);
	~unary_operator_right_expression();
	expression* get_operand();
	void set_operand(expression* operand);
	std::string get_operator();
	void set_operator(std::string operator_name);
	std::string to_string();
	std::vector<expression**>* get_children();
	void accept(ast_visitor* visitor);
};

class const_boolean_expression: public expression {
	bool value;
public:
	const_boolean_expression(bool value);
	bool get_value();
	void set_value(bool value);
	std::string to_string();
	void accept(ast_visitor* visitor);
};

class const_integer_expression: public expression {
	int value;
	radix rad;
public:
	const_integer_expression(int value);
	const_integer_expression(int value, radix rad);
	int get_value();
	void set_value(int value);
	radix get_radix();
	void set_radix(radix rad);
	std::string to_string();
	void accept(ast_visitor* visitor);
};

class const_float_expression: public expression {
	float value;
public:
	const_float_expression(float value);
	float get_value();
	void set_value(float value);
	std::string to_string();
	void accept(ast_visitor* visitor);
};

class const_double_expression: public expression {
	double value;
public:
	const_double_expression(double value);
	double get_value();
	void set_value(double value);
	std::string to_string();
	void accept(ast_visitor* visitor);
};

class const_string_expression: public expression {
	std::string value;
public:
	const_string_expression(std::string value);
	std::string get_value();
	void set_value(std::string value);
	std::string to_string();
	void accept(ast_visitor* visitor);
};

class cast_expression: public expression {
	type_ref target_type;
	expression* operand;
public:
	cast_expression(type_ref target_type, expression* operand);
	~cast_expression();
	type_ref get_target_type();
	void set_target_type(type_ref target_type);
	expression* get_operand();
	void set_operand(expression* operand);
	std::string to_string();
	std::vector<expression**>* get_children();
	void accept(ast_visitor* visitor);
};

class array_expression: public expression {
	expression* target;
	std::vector<expression*>* indices;
public:
	array_expression(expression* target, std::vector<expression*>* index);
	~array_expression();
	expression* get_target();
	void set_target(expression* target);
	std::vector<expression*>* get_indices();
	std::string to_string();
	std::vector<expression**>* get_children();
	void accept(ast_visitor* visitor);
};

class statement {
	statement_kind kind;
	statement* parent_statement = nullptr;
	ast_node* parent_node = nullptr;
protected:
	statement(statement_kind kind);
public:
	virtual ~statement();
	statement_kind get_statement_kind();
	bool is_of_statement_kind(statement_kind kind);
	statement* get_parent_statement();
	void set_parent_statement(statement* parent);
	ast_node* get_parent_node();
	void set_parent_node(ast_node* parent);
	virtual std::string to_string();
	virtual std::vector<statement**>* get_child_statements();
	virtual std::vector<expression**>* get_child_expressions();
	virtual void accept(ast_visitor* visitor);
};

class block_statement: public statement {
	std::vector<statement*>* children;
public:
	block_statement(std::vector<statement*>* children);
	~block_statement();
	std::vector<statement*>* get_children();
	std::string to_string();
	std::vector<statement**>* get_child_statements();
	void accept(ast_visitor* visitor);
};

class variable_declaration_statement: public statement {
	std::set<modifier>* modifiers;
	type_ref type;
	std::string name;
	expression* initialization_expression;
public:
	variable_declaration_statement(std::set<modifier>* modifiers, type_ref type,
			std::string name, expression* initialization_expression);
	~variable_declaration_statement();
	std::set<modifier>* get_modifiers();
	type_ref get_type();
	void set_type(type_ref type);
	std::string get_name();
	void set_name(std::string name);
	expression* get_initialization_expression();
	void set_initialization_expression(expression* initialization_expression);
	std::string to_string();
	std::vector<expression**>* get_child_expressions();
	void accept(ast_visitor* visitor);
};

class assignment_statement: public statement {
	expression* lhs;
	std::string assignment_operator;
	expression* rhs;
public:
	assignment_statement(expression* lhs, std::string assignment_operator,
			expression* rhs);
	~assignment_statement();
	expression* get_lhs();
	void set_lhs(expression* lhs);
	std::string get_assignment_operator();
	void set_assignment_operator(std::string assignment_operator);
	expression* get_rhs();
	void set_rhs(expression* rhs);
	std::string to_string();
	std::vector<expression**>* get_child_expressions();
	void accept(ast_visitor* visitor);
};

class if_statement: public statement {
	expression* condition;
	statement* if_clause;
	statement* else_clause;
public:
	if_statement(expression* condition, statement* if_clause);
	if_statement(expression* condition, statement* if_clause,
			statement* else_clause);
	~if_statement();
	expression* get_condition();
	void set_condition(expression* condition);
	statement* get_if_clause();
	void set_if_clause(statement* if_clause);
	bool has_else_clause();
	statement* get_else_clause();
	void set_else_clause(statement* else_clause);
	std::string to_string();
	std::vector<statement**>* get_child_statements();
	std::vector<expression**>* get_child_expressions();
	void accept(ast_visitor* visitor);
};

class while_statement: public statement {
	expression* condition;
	statement* while_clause;
public:
	while_statement(expression* condition, statement* while_clause);
	~while_statement();
	expression* get_condition();
	void set_condition(expression* condition);
	statement* get_while_clause();
	void set_while_clause(statement* while_clause);
	std::string to_string();
	std::vector<statement**>* get_child_statements();
	std::vector<expression**>* get_child_expressions();
	void accept(ast_visitor* visitor);
};

class do_while_statement: public statement {
	statement* do_while_clause;
	expression* condition;
public:
	do_while_statement(statement* do_while_clause, expression* condition);
	~do_while_statement();
	statement* get_do_while_clause();
	void set_do_while_clause(statement* do_while_clause);
	expression* get_condition();
	void set_condition(expression* condition);
	std::string to_string();
	std::vector<statement**>* get_child_statements();
	std::vector<expression**>* get_child_expressions();
	void accept(ast_visitor* visitor);
};

class for_statement: public statement {
	statement* initializer;
	expression* condition;
	statement* increment;
	statement* for_clause;
public:
	for_statement(statement* initializer, expression* condition,
			statement* increment, statement* for_clause);
	~for_statement();
	bool has_initializer();
	statement* get_initializer();
	void set_initializer(statement* initializer);
	bool has_condition();
	expression* get_condition();
	void set_condition(expression* condition);
	bool has_increment();
	statement* get_increment();
	void set_increment(statement* increment);
	statement* get_for_clause();
	void set_for_clause(statement* for_clause);
	std::string to_string();
	std::vector<statement**>* get_child_statements();
	std::vector<expression**>* get_child_expressions();
	void accept(ast_visitor* visitor);
};

class forever_statement: public statement {
	statement* forever_clause;
public:
	forever_statement(statement* forever_clause);
	~forever_statement();
	statement* get_forever_clause();
	void set_forever_clause(statement* forever_clause);
	std::string to_string();
	std::vector<statement**>* get_child_statements();
	void accept(ast_visitor* visitor);
};

class repeat_statement: public statement {
	expression* times;
	statement* repeat_clause;
public:
	repeat_statement(expression* times, statement* repeat_clause);
	~repeat_statement();
	expression* get_times();
	void set_times(expression* times);
	statement* get_repeat_clause();
	void set_repeat_clause(statement* repeat_clause);
	std::string to_string();
	std::vector<statement**>* get_child_statements();
	std::vector<expression**>* get_child_expressions();
	void accept(ast_visitor* visitor);
};

class return_statement: public statement {
	expression* operand;
public:
	return_statement(expression* operand);
	~return_statement();
	expression* get_operand();
	void set_operand(expression* operand);
	std::string to_string();
	std::vector<expression**>* get_child_expressions();
	void accept(ast_visitor* visitor);
};

class expression_statement: public statement {
	expression* expr;
public:
	expression_statement(expression* expr);
	~expression_statement();
	expression* get_expression();
	void set_expression(expression* expr);
	std::string to_string();
	std::vector<expression**>* get_child_expressions();
	void accept(ast_visitor* visitor);
};

class ast_node {
	ast_node_kind kind;
	ast_node* parent_node = nullptr;
protected:
	ast_node(ast_node_kind kind);
public:
	virtual ~ast_node();
	ast_node_kind get_ast_node_kind();
	bool is_of_ast_node_kind(ast_node_kind kind);
	ast_node* get_parent_node();
	void set_parent_node(ast_node* parent);
	virtual std::string to_string();
	virtual std::vector<ast_node**>* get_child_nodes();
	virtual std::vector<statement**>* get_child_statements();
	virtual std::vector<expression**>* get_child_expressions();
	virtual void accept(ast_visitor* visitor);
};

class module_node: public ast_node {
	std::string namespace_name;
	std::vector<ast_node*>* children;
public:
	module_node(std::string namespace_name, std::vector<ast_node*>* children);
	module_node(std::vector<ast_node*>* children);
	~module_node();
	std::string get_namespace();
	void set_namespace(std::string namespace_name);
	std::vector<ast_node*>* get_children();
	std::string to_string();
	std::vector<ast_node**>* get_child_nodes();
	void accept(ast_visitor* visitor);
};

class field_node: public ast_node {
	std::set<modifier>* modifiers;
	type_ref type;
	std::string name;
	expression* initialization_expression;
public:
	field_node(std::set<modifier>* modifiers, type_ref type, std::string name);
	field_node(std::set<modifier>* modifiers, type_ref type, std::string name,
			expression* initialization_expression);
	~field_node();
	std::set<modifier>* get_modifiers();
	type_ref get_type();
	void set_type(type_ref type);
	std::string get_name();
	void set_name(std::string name);
	bool has_initialization_expression();
	expression* get_initialization_expression();
	void set_initialization_expression(expression* initialization_expression);
	std::string to_string();
	std::vector<expression**>* get_child_expressions();
	void accept(ast_visitor* visitor);
};

class function_node: public ast_node {
	std::set<modifier>* modifiers;
	type_ref return_type;
	std::string name;
	std::vector<field_node*>* parameters;
	statement* body;
public:
	function_node(std::set<modifier>* modifiers, type_ref return_type,
			std::string name, std::vector<field_node*>* parameters,
			statement* body);
	~function_node();
	std::set<modifier>* get_modifiers();
	type_ref get_return_type();
	void set_return_type(type_ref return_type);
	std::string get_name();
	void set_name(std::string name);
	std::vector<field_node*>* get_parameters();
	statement* get_body();
	void set_body(statement* body);
	std::string to_string();
	std::vector<statement**>* get_child_statements();
	std::vector<expression**>* get_child_expressions();
	void accept(ast_visitor* visitor);
};

class ast_visitor {
public:
	virtual ~ast_visitor();

	virtual void visit_identifier_expression(identifier_expression* expr);
	virtual void visit_parenthesized_expression(parenthesized_expression* expr);
	virtual void visit_call_expression(call_expression* expr);
	virtual void visit_namespace_expression(namespace_expression* expr);
	virtual void visit_operator_expression(operator_expression* expr);
	virtual void visit_unary_operator_left_expression(
			unary_operator_left_expression* expr);
	virtual void visit_unary_operator_right_expression(
			unary_operator_right_expression* expr);
	virtual void visit_const_boolean_expression(const_boolean_expression* expr);
	virtual void visit_const_integer_expression(const_integer_expression* expr);
	virtual void visit_const_float_expression(const_float_expression* expr);
	virtual void visit_const_double_expression(const_double_expression* expr);
	virtual void visit_const_string_expression(const_string_expression* expr);
	virtual void visit_cast_expression(cast_expression* expr);
	virtual void visit_array_expression(array_expression* expr);

	virtual void visit_block_statement(block_statement* stmt);
	virtual void visit_variable_declaration_statement(
			variable_declaration_statement* stmt);
	virtual void visit_assignment_statement(assignment_statement* stmt);
	virtual void visit_if_statement(if_statement* stmt);
	virtual void visit_while_statement(while_statement* stmt);
	virtual void visit_do_while_statement(do_while_statement* stmt);
	virtual void visit_for_statement(for_statement* stmt);
	virtual void visit_forever_statement(forever_statement* stmt);
	virtual void visit_repeat_statement(repeat_statement* stmt);
	virtual void visit_return_statement(return_statement* stmt);
	virtual void visit_expression_statement(expression_statement* stmt);

	virtual void visit_module_node(module_node* node);
	virtual void visit_field_node(field_node* node);
	virtual void visit_function_node(function_node* node);

	virtual void visit_expression(expression* expr);
	virtual void visit_statement(statement* stmt);
	virtual void visit_ast_node(ast_node* node);

	void visit_all(std::vector<ast_node*>* nodes);
};

#endif /* RELEASE_CROSSLANG_AST_HPP_ */
