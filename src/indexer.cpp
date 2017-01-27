/*
 *   Author: Earthcomputer
 */

#include "indexer.hpp"

indexer::field_index::field_index(bool global, std::string name,
		ast::type_ref type) :
		global(global), name(name), type(type) {
}
bool indexer::field_index::is_global() {
	return global;
}
std::string indexer::field_index::get_name() {
	return name;
}
ast::type_ref indexer::field_index::get_type() {
	return type;
}

indexer::function_index::function_index(bool global, std::string name,
		ast::type_ref return_type, std::vector<ast::type_ref>* parameters) :
		global(global), name(name), return_type(return_type), parameters(
				parameters) {
}
bool indexer::function_index::is_global() {
	return global;
}
std::string indexer::function_index::get_name() {
	return name;
}
ast::type_ref indexer::function_index::get_return_type() {
	return return_type;
}
std::vector<ast::type_ref>* indexer::function_index::get_parameter_types() {
	return parameters;
}

indexer::module_index::module_index(std::set<indexer::module_index*>* modules,
		std::set<indexer::field_index*>* fields,
		std::set<indexer::function_index*>* functions) :
		name_exists(false), name(), modules(modules), fields(fields), functions(
				functions) {
}
indexer::module_index::module_index(std::string name,
		std::set<indexer::module_index*>* modules,
		std::set<indexer::field_index*>* fields,
		std::set<indexer::function_index*>* functions) :
		name_exists(true), name(name), modules(modules), fields(fields), functions(
				functions) {
}
indexer::module_index::~module_index() {
	for (indexer::module_index* ns : *modules) {
		delete ns;
	}
	delete modules;
	for (indexer::field_index* field : *fields) {
		delete field;
	}
	delete fields;
	for (indexer::function_index* function : *functions) {
		delete function;
	}
	delete functions;
}
bool indexer::module_index::has_name() {
	return name_exists;
}
std::string indexer::module_index::get_name() {
	return name;
}
std::set<indexer::module_index*>* indexer::module_index::get_modules() {
	return modules;
}
std::set<indexer::field_index*>* indexer::module_index::get_fields() {
	return fields;
}
std::set<indexer::function_index*>* indexer::module_index::get_functions() {
	return functions;
}
void indexer::module_index::add_field(indexer::field_index* field) {
	for (indexer::field_index* existing_field : *fields) {
		if (existing_field->get_name() == field->get_name()) {
			throw indexer::indexer_exception(
					("Duplicate field " + field->get_name()).c_str());
		}
	}
	fields->insert(field);
}
void indexer::module_index::add_function(indexer::function_index* function) {
	for (indexer::function_index* existing_function : *functions) {
		bool same_name = existing_function->get_name() == function->get_name();
		std::vector<ast::type_ref>* existing_param_types =
				existing_function->get_parameter_types();
		std::vector<ast::type_ref>* param_types =
				function->get_parameter_types();
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
					throw indexer::indexer_exception(
							("Duplicate function " + function->get_name()).c_str());
				}
			}
		}
	}
	functions->insert(function);
}
void indexer::module_index::add_module(indexer::module_index* ns) {
	if (ns->has_name()) {
		for (indexer::module_index* existing_namespace : *modules) {
			if (existing_namespace->has_name()) {
				if (existing_namespace->get_name() == ns->get_name()) {
					throw indexer::indexer_exception(
							("Duplicate namespace " + ns->get_name()).c_str());
				}
			}
		}
	}
	modules->insert(ns);
}

indexer::indexer_exception::indexer_exception(const char* desc) :
		desc(desc) {
}

indexer::indexer_exception::~indexer_exception() {
	delete desc;
}

const char* indexer::indexer_exception::what() {
	return desc;
}

class indexer_visitor: public ast::ast_visitor {
	std::vector<indexer::module_index*> module_stack = std::vector<
			indexer::module_index*>(1,
			new indexer::module_index(new std::set<indexer::module_index*>,
					new std::set<indexer::field_index*>,
					new std::set<indexer::function_index*>));
public:
	void visit_field_node(ast::field_node* field) {
		bool global = field->get_modifiers()->find(ast::modifier::GLOBAL)
				!= field->get_modifiers()->end();
		module_stack.back()->add_field(
				new indexer::field_index(global, field->get_name(), field->get_type()));
	}
	void visit_function_node(ast::function_node* func) {
		bool global = func->get_modifiers()->find(ast::modifier::GLOBAL)
				!= func->get_modifiers()->end();
		std::vector<ast::type_ref>* param_types = new std::vector<ast::type_ref>;
		for (ast::field_node* field : *(func->get_parameters())) {
			param_types->push_back(field->get_type());
		}
		module_stack.back()->add_function(
				new indexer::function_index(global, func->get_name(),
						func->get_return_type(), param_types));
	}
	void visit_module_node(ast::module_node* module) {
		std::string ns = module->get_namespace();
		indexer::module_index * idx;
		if (ns.empty()) {
			idx = new indexer::module_index(new std::set<indexer::module_index*>,
					new std::set<indexer::field_index*>, new std::set<indexer::function_index*>);
		} else {
			idx = new indexer::module_index(ns, new std::set<indexer::module_index*>,
					new std::set<indexer::field_index*>, new std::set<indexer::function_index*>);
		}
		module_stack.back()->add_module(idx);
		module_stack.push_back(idx);
	}
	void visit_ast_node(ast::ast_node* node) {
		ast::ast_visitor::visit_ast_node(node);
		if (node->is_of_ast_node_kind(ast::ast_node_kind::MODULE)) {
			module_stack.pop_back();
		}
	}
	indexer::index* get_top_index() {
		return module_stack.back();
	}
};

indexer::index* indexer::index_ast_tree(std::vector<ast::ast_node*>* tree) {
	indexer_visitor* visitor = new indexer_visitor;
	visitor->visit_all(tree);
	indexer::index* ret = visitor->get_top_index();
	delete visitor;
	return ret;
}
