/*
 *   Author: Earthcomputer
 */

#ifndef INDEXER_HPP_
#define INDEXER_HPP_

#include <set>
#include <vector>
#include "crosslang_ast.hpp"

class field_index {
	bool global;
	std::string name;
	type_ref type;
public:
	field_index(bool global, std::string name, type_ref type);
	bool is_global();
	std::string get_name();
	type_ref get_type();
};

class function_index {
	bool global;
	std::string name;
	type_ref return_type;
	std::vector<type_ref>* parameters;
public:
	function_index(bool global, std::string name, type_ref return_type,
			std::vector<type_ref>* parameters);
	bool is_global();
	std::string get_name();
	type_ref get_return_type();
	std::vector<type_ref>* get_parameter_types();
};

class module_index {
	bool name_exists;
	std::string name;
	std::set<module_index*>* modules;
	std::set<field_index*>* fields;
	std::set<function_index*>* functions;
public:
	module_index(std::set<module_index*>* modules,
			std::set<field_index*>* fields,
			std::set<function_index*>* functions);
	module_index(std::string name, std::set<module_index*>* modules,
			std::set<field_index*>* fields,
			std::set<function_index*>* functions);
	~module_index();
	bool has_name();
	std::string get_name();
	std::set<module_index*>* get_modules();
	std::set<field_index*>* get_fields();
	std::set<function_index*>* get_functions();
	void add_module(module_index* ns);
	void add_field(field_index* field);
	void add_function(function_index* function);
};

typedef module_index index;

class indexer_exception: public std::exception {
	const char* desc;
public:
	indexer_exception(const char* desc);
	~indexer_exception() throw ();
	const char* what();
};

index* index_ast_tree(std::vector<ast_node*>* tree);

#endif /* INDEXER_HPP_ */
