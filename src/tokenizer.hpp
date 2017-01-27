/*
 *      Author: Earthcomputer
 */

#ifndef TOKENIZER_HPP
#define TOKENIZER_HPP

#include <string>
#include <vector>

namespace tokenizer {

class tokenizer_exception: public std::exception {
	const char* description;
	int pos;
public:
	tokenizer_exception(const char* what, int pos);
	~tokenizer_exception() throw ();
	const char* what();
	int get_pos();
};

enum class token_kind {
	IDENTIFIER,
	NUMBER,
	OPERATOR,
	SINGLE_QUOTED_STRING,
	DOUBLE_QUOTED_STRING,
	END_OF_FILE
};

struct token {
	token_kind kind;
	std::string text;
	int pos;
};

void tokenize(std::string in, std::vector<token>& tokens,
		std::vector<int>& line_breaks);

std::string read_input_stream(std::istream& in);

}

#endif
