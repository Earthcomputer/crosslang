/*
 *      Author: Earthcomputer
 */

#include <fstream>
#include <vector>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <map>

#include "tokenizer.hpp"
#include "crosslang_ast.hpp"
#include "parser.hpp"
#include "indexer.hpp"
#include "errorcodes.hpp"

int get_line_number(std::vector<int> line_numbers, int pos) {
	int size = line_numbers.size();
	if (size == 0) {
		return 1;
	}
	for (int i = 0; i < size; i++) {
		if (line_numbers[i] > pos) {
			return i + 1;
		}
	}
	return size;
}

void print_field_index(indexer::field_index* idx) {
	std::cout << "Field: " << idx->get_type().to_string() << " "
			<< idx->get_name() << std::endl;
}

void print_function_index(indexer::function_index* idx) {
	std::string params = "(";
	bool first = true;
	for (ast::type_ref& type : *(idx->get_parameter_types())) {
		if (!first) {
			params += ", ";
		}
		first = false;
		params += type.to_string();
	}
	params += ")";
	std::cout << "Function: " << idx->get_return_type().to_string() << " "
			<< idx->get_name() << " " << params << std::endl;
}

void print_module_index(indexer::module_index* idx) {
	std::string str = "Module";
	if (idx->has_name()) {
		str += " " + idx->get_name();
	}
	str += ":";
	std::cout << str << std::endl;
	std::cout << "{" << std::endl;
	for (indexer::module_index* submodule : *(idx->get_modules())) {
		print_module_index(submodule);
	}
	for (indexer::field_index* subfield : *(idx->get_fields())) {
		print_field_index(subfield);
	}
	for (indexer::function_index* subfunction : *(idx->get_functions())) {
		print_function_index(subfunction);
	}
	std::cout << "}" << std::endl;
}

void print_random_witty_comment() {
	std::cerr << "-----------------------" << std::endl;
	const int LENGTH = 4;
	const std::string comments[LENGTH] = {
			"And you call yourself a programmer?",
			"Unlike you, I don't make mitsakes",
			"What on Earth were you thinking when you typed that?",
			"Wrong! Try again!" };
	std::cerr << comments[rand() % LENGTH] << std::endl;
}

int main(const int argc, char* argv[]) {
	srand(time(NULL));

	std::vector<std::string> args;
	for (int i = 1; i < argc; i++) {
		std::string str(argv[i]);
		args.push_back(str);
	}

	std::map<std::string, std::vector<ast::ast_node * > * > ast_by_filename;
	indexer::index* dictionary = new indexer::index;

	for (std::string file : args) {
		std::ifstream in;
		in.open(file);
		if (!in.good()) {
			std::cerr << "Failed to open file " << file << std::endl;
			return ERR_FOPEN_FAILED;
		}
		std::string text = tokenizer::read_input_stream(in);
		in.close();

		std::vector<tokenizer::token> tokens;
		std::vector<int> line_numbers;
		try {
			tokenizer::tokenize(text, tokens, line_numbers);

			std::vector<ast::ast_node*>* nodes = parser::parse(tokens);

			indexer::index_ast_tree(nodes, dictionary);
		} catch (tokenizer::tokenizer_exception& e) {
			std::cerr << "COMPILATION FAILED WHILE TOKENIZING!" << std::endl;
			std::cerr
					<< "This means the compiler failed to split the file up into tokens (words)."
					<< std::endl;
			std::cerr
					<< "This is normally caused by an unclosed string/comment."
					<< std::endl;
			std::cerr << "File: " << file << std::endl;
			std::cerr << "Message: " << e.what() << std::endl;
			print_random_witty_comment();
			return ERR_TOKENIZE_FAILED;
		} catch (parser::parser_exception& e) {
			std::cerr << "COMPILATION FAILED WHILE PARSING!" << std::endl;
			std::cerr
					<< "This means the compiler was unable to deduce the structure of the code."
					<< std::endl;
			std::cerr << "This is normally caused by a syntax error."
					<< std::endl;
			std::cerr << "File: " << file << std::endl;
			std::cerr << "Message: " << e.what() << std::endl;
			std::cerr << "Line number: "
					<< get_line_number(line_numbers, e.get_pos()) << std::endl;
			print_random_witty_comment();
			return ERR_TOKENIZE_FAILED;
		} catch (indexer::indexer_exception& e) {
			std::cerr << "COMPILATION FAILED WHILE INDEXING!" << std::endl;
			std::cerr
					<< "This occurs when the compiler is trying to build an index (dictionary) of fields, functions, etc."
					<< std::endl;
			std::cerr << "File: " << file << std::endl;
			std::cerr << "Message: " << e.what() << std::endl;
			print_random_witty_comment();
			return ERR_INDEX_FAILED;
		}
	}

	return SUCCESS;
}

