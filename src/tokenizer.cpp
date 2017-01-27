/*
 *      Author: Earthcomputer
 */

#include <iostream>
#include <string>
#include <vector>
#include "tokenizer.hpp"

tokenizer::tokenizer_exception::tokenizer_exception(const char* what, int pos) :
		description(what), pos(pos) {
}
tokenizer::tokenizer_exception::~tokenizer_exception() throw () {
	delete description;
}
const char* tokenizer::tokenizer_exception::what() {
	return description;
}
int tokenizer::tokenizer_exception::get_pos() {
	return pos;
}

std::string multichar_operators[] = { ">>", "<<", "+=", "-=", "*=", "/=", "%=",
		"&=", "|=", "^=", ">>=", "<<=", "&&", "||", "^^", "==", "!=", ">=",
		"<=", "++", "--", "->", "::" };

void tokenizer::tokenize(std::string in, std::vector<tokenizer::token>& tokens,
		std::vector<int>& line_breaks) {
	bool in_token = false;
	bool in_singleline_comment = false;
	bool in_multiline_comment = false;
	bool is_escaped = false;
	char last_char = -1;
	tokenizer::token current_token;
	int pos = 0;

	for (const auto& c : in) {
		// check for newline and do newline stuff
		if (c == '\n') {
			// add line number index to list
			line_breaks.push_back(pos);
			// close any single-line comments
			in_singleline_comment = false;
			// we've reached the end of the current token
			if (in_token) {
				in_token = false;
				tokens.push_back(current_token);
			}
			goto next_iteration;
		}

		// skip if commented out
		if (in_multiline_comment) {
			if (c == '/' && last_char == '*') {
				in_multiline_comment = false;
			}
			goto next_iteration;
		}
		if (in_singleline_comment) {
			goto next_iteration;
		}

		// check for token-specific stuff
		if (in_token) {
			// in this switch statement, we goto next_iteration if
			// the token is to continue, and break if the token is to end
			switch (current_token.kind) {
			case tokenizer::token_kind::IDENTIFIER: {
				if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')
						|| (c >= '0' && c <= '9') || (c == '_')) {
					current_token.text += c;
					goto next_iteration;
				}
				break;
			}
			case tokenizer::token_kind::NUMBER: {
				if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F')
						|| (c >= 'a' && c <= 'f') || (c == '.') || (c == 'x')
						|| (c == 'X')) {
					current_token.text += c;
					goto next_iteration;
				}
				break;
			}
			case tokenizer::token_kind::OPERATOR: {
				// check if a multichar operator could include this char
				std::string potential_token_text = current_token.text + c;
				bool could_be_multichar = false;
				for (const auto& multichar_operator : multichar_operators) {
					if (!multichar_operator.compare(0,
							potential_token_text.length(),
							potential_token_text)) {
						could_be_multichar = true;
						break;
					}
				}
				if (could_be_multichar) {
					current_token.text += c;
					goto next_iteration;
				}
				// check if we've ended on a valid token by checking against the
				// list of multichar operators
				if (current_token.text.length() != 1) {
					could_be_multichar = false;
					for (const auto& multichar_operator : multichar_operators) {
						if (multichar_operator == current_token.text) {
							could_be_multichar = true;
							break;
						}
					}
					if (!could_be_multichar) {
						throw tokenizer::tokenizer_exception(
								"Unable to parse multichar operator", pos);
					}
				}
				break;
			}
			case tokenizer::token_kind::SINGLE_QUOTED_STRING:
			case tokenizer::token_kind::DOUBLE_QUOTED_STRING: {
				if (c == '\\') {
					is_escaped = !is_escaped;
				} else {
					bool has_ended_string = false;
					if (((current_token.kind
							== tokenizer::token_kind::SINGLE_QUOTED_STRING
							&& c == '\'')
							|| (current_token.kind
									== tokenizer::token_kind::DOUBLE_QUOTED_STRING
									&& c == '"')) && !is_escaped) {
						has_ended_string = true;
					}
					current_token.text += c;
					is_escaped = false;
					if (has_ended_string) {
						// have to end string tokens differently because
						// they end on the last character of the token
						// (the closing quote) rather than the first
						// invalid character which could be of the next token
						tokens.push_back(current_token);
						in_token = false;
					}
				}
				goto next_iteration;
			}
			case tokenizer::token_kind::END_OF_FILE:
				throw tokenizer::tokenizer_exception(
						"A token's kind should never be EOF!",
						current_token.pos);
			}
			// do end-of-token stuff
			tokens.push_back(current_token);
			in_token = false;
		}

		// skip whitespace
		if (c <= ' ') {
			goto next_iteration;
		}

		// check for start of comments
		if (c == '#') {
			in_singleline_comment = true;
			goto next_iteration;
		}
		if (c == '/' && last_char == '/') {
			// the last char would have caused a false token, remove it
			tokens.pop_back();
			in_singleline_comment = true;
			goto next_iteration;
		}
		if (c == '*' && last_char == '/') {
			// the last char would have caused a false token, remove it
			tokens.pop_back();
			in_multiline_comment = true;
			goto next_iteration;
		}

		// check for start of different token types
		{
			bool started_token = false;
			tokenizer::token_kind kind;
			if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'z')
					|| (c == '_')) {
				started_token = true;
				kind = tokenizer::token_kind::IDENTIFIER;
			} else if (c >= '0' && c <= '9') {
				started_token = true;
				kind = tokenizer::token_kind::NUMBER;
			} else if (c == '\'') {
				started_token = true;
				kind = tokenizer::token_kind::SINGLE_QUOTED_STRING;
			} else if (c == '"') {
				started_token = true;
				kind = tokenizer::token_kind::DOUBLE_QUOTED_STRING;
			} else {
				started_token = true;
				kind = tokenizer::token_kind::OPERATOR;
			}
			if (started_token) {
				in_token = true;
				current_token.kind = kind;
				current_token.text = c;
				current_token.pos = pos;
				goto next_iteration;
			}
		}

		next_iteration: last_char = c;
		pos++;
	}
	// check end of file stuff
	if (in_multiline_comment) {
		throw tokenizer::tokenizer_exception(
				"Reached the end of the file before the end of a multiline comment",
				pos);
	}
	if (in_token) {
		tokenizer::token_kind kind = current_token.kind;
		if (kind == tokenizer::token_kind::SINGLE_QUOTED_STRING
				|| kind == tokenizer::token_kind::DOUBLE_QUOTED_STRING) {
			throw tokenizer::tokenizer_exception(
					"Reached the end of the file before the end of a string",
					pos);
		}
		if (kind == tokenizer::token_kind::OPERATOR
				&& current_token.text.length() != 1) {
			bool is_multichar_operator = false;
			for (const auto& multichar_operator : multichar_operators) {
				if (current_token.text == multichar_operator) {
					is_multichar_operator = true;
					break;
				}
			}
			if (!is_multichar_operator) {
				throw tokenizer::tokenizer_exception(
						"Unable to parse multichar operator", pos);
			}
		}
		tokens.push_back(current_token);
		in_token = false;
	}
}

std::string tokenizer::read_input_stream(std::istream& in) {
	std::string tmp;
	std::string ret;
	while (std::getline(in, tmp)) {
		ret += tmp;
		ret += '\n';
	}
	return ret;
}
