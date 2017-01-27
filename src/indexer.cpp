/*
 *   Author: Earthcomputer
 */

#include "indexer.hpp"

field_index::field_index(bool global, std::string name, type_ref type) :
		global(global), name(name), type(type) {
}
bool field_index::is_global() {
	return global;
}
std::string field_index::get_name() {
	return name;
}
type_ref field_index::get_type() {
	return type;
}

function_index::function_index(bool global, std::string name,
		type_ref return_type, std::vector<type_ref>* parameters) :
		global(global), name(name), return_type(return_type), parameters(
				parameters) {
}
bool function_index::is_global() {
	return global;
}
std::string function_index::get_name() {
	return name;
}
type_ref function_index::get_return_type() {
	return return_type;
}
std::vector<type_ref>* function_index::get_parameter_types() {
	return parameters;
}

module_index::module_index(std::set<module_index*>* modules,
		std::set<field_index*>* fields, std::set<function_index*>* functions) :
		name_exists(false), name(), modules(modules), fields(fields), functions(
				functions) {
}
module_index::module_index(std::string name, std::set<module_index*>* modules,
		std::set<field_index*>* fields, std::set<function_index*>* functions) :
		name_exists(true), name(name), modules(modules), fields(fields), functions(
				functions) {
}
module_index::~module_index() {
	for (module_index* ns : *modules) {
		delete ns;
	}
	delete modules;
	for (field_index* field : *fields) {
		delete field;
	}
	delete fields;
	for (function_index* function : *functions) {
		delete function;
	}
	delete functions;
}
bool module_index::has_name() {
	return name_exists;
}
std::string module_index::get_name() {
	return name;
}
std::set<module_index*>* module_index::get_modules() {
	return modules;
}
std::set<field_index*>* module_index::get_fields() {
	return fields;
}
std::set<function_index*>* module_index::get_functions() {
	return functions;
}
void module_index::add_field(field_index* field) {
	for (field_index* existing_field : *fields) {
		if (existing_field->get_name() == field->get_name()) {
			throw indexer_exception(
					("Duplicate field " + field->get_name()).c_str());
		}
	}
	fields->insert(field);
}
void module_index::add_function(function_index* function) {
	for (function_index* existing_function : *functions) {
		bool same_name = existing_function->get_name() == function->get_name();
		std::vector<type_ref>* existing_param_types =
				existing_function->get_parameter_types();
		std::vector<type_ref>* param_types = function->get_parameter_types();
		if (same_name) {
			if (existing_param_types->size() == param_types->size()) {
				bool all_same = true;
				for (int i = 0, e = param_types->size(); i < e; i++) {
					if ((*existing_param_types)[i] != (*param_types)[i]) {
						all_same = false;
						break;
					}
				}
				if (all_same) {
					throw indexer_exception(
							("Duplicate function " + function->get_name()).c_str());
				}
			}
		}
	}
	functions->insert(function);
}
void module_index::add_module(module_index* ns) {
	if (ns->has_name()) {
		for (module_index* existing_namespace : *modules) {
			if (existing_namespace->has_name()) {
				if (existing_namespace->get_name() == ns->get_name()) {
					throw indexer_exception(
							("Duplicate namespace " + ns->get_name()).c_str());
				}
			}
		}
	}
	modules->insert(ns);
}

indexer_exception::indexer_exception(const char* desc) :
		desc(desc) {
}

indexer_exception::~indexer_exception() {
	delete desc;
}

const char* indexer_exception::what() {
	return desc;
}

class indexer_visitor: public ast_visitor {
	std::vector<module_index*> module_stack = std::vector<module_index*>(1,
			new module_index(new std::set<module_index*>,
					new std::set<field_index*>, new std::set<function_index*>));
public:
	void visit_field_node(field_node* field) {
		bool global = field->get_modifiers()->find(modifier::GLOBAL)
				!= field->get_modifiers()->end();
		module_stack.back()->add_field(
				new field_index(global, field->get_name(), field->get_type()));
	}
	void visit_function_node(function_node* func) {
		bool global = func->get_modifiers()->find(modifier::GLOBAL)
				!= func->get_modifiers()->end();
		std::vector<type_ref>* param_types = new std::vector<type_ref>;
		for (field_node* field : *(func->get_parameters())) {
			param_types->push_back(field->get_type());
		}
		module_stack.back()->add_function(
				new function_index(global, func->get_name(),
						func->get_return_type(), param_types));
	}
	void visit_module_node(module_node* module) {
		std::string ns = module->get_namespace();
		module_index * idx;
		if (ns.empty()) {
			idx = new module_index(new std::set<module_index*>,
					new std::set<field_index*>, new std::set<function_index*>);
		} else {
			idx = new module_index(ns, new std::set<module_index*>,
					new std::set<field_index*>, new std::set<function_index*>);
		}
		module_stack.back()->add_module(idx);
		module_stack.push_back(idx);
	}
	void visit_ast_node(ast_node* node) {
		ast_visitor::visit_ast_node(node);
		if (node->is_of_ast_node_kind(ast_node_kind::MODULE)) {
			module_stack.pop_back();
		}
	}
	index* get_top_index() {
		return module_stack.back();
	}
};

index* index_ast_tree(std::vector<ast_node*>* tree) {
	indexer_visitor* visitor = new indexer_visitor;
	visitor->visit_all(tree);
	index* ret = visitor->get_top_index();
	delete visitor;
	return ret;
}
