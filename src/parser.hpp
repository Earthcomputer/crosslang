/*
 *      Author: Earthcomputer
 */

#ifndef PARSER_HPP_
#define PARSER_HPP_

#include <vector>
#include "tokenizer.hpp"
#include "crosslang_ast.hpp"

namespace parser {

class parser_exception: public std::exception {
	int pos;
	const char* desc;
public:
	parser_exception(const char* desc, int pos);
	~parser_exception() throw ();
	const char* what();
	int get_pos();
};

std::vector<ast::ast_node*>* parse(std::vector<tokenizer::token> tokens);

}
#endif /* PARSER_HPP_ */
