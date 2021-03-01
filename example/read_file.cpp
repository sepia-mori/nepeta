// Part of the nepeta project.
// Original source: https://git.sepiamori.com/sepia-mori/nepeta
//
//          Copyright Sepia Mori AS 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

// This example loads a file from the command line, outputting any errors,
// and finally outputting a string-representation of the same document back to the console.
#include <iostream>
#include <fstream>
#include <vector>
#include <map>

#include "nepeta.h"

// Outputs nepeta errors detected during parsing
void print_error(nepeta::parser_error error, char ch, unsigned int line, unsigned int column)
{
	static const std::map<nepeta::parser_error, std::string> error_mapping = {
		{nepeta::parser_error::bad_codec,                    "Error: Bad codec to block"},
		{nepeta::parser_error::block_not_closed,             "Error: Block not closed"},
		{nepeta::parser_error::comment_not_closed,           "Error: Comment not closed"},
		{nepeta::parser_error::illegal_character,            "Error: Illegal character"},
		{nepeta::parser_error::node_not_closed,              "Error: Node not closed"},
		{nepeta::parser_error::recursion_limit_reached,      "Error: Node recursion limit reached"},
		{nepeta::parser_error::string_not_closed,            "Error: String was not closed"},
		{nepeta::parser_error::too_may_node_closing_markers, "Error: Too many node closing markers ('#')"},
		{nepeta::parser_error::bad_block_close,              "Warning: Text block was attempted closed at unexpected indentation ('}')"},
		{nepeta::parser_error::invalid_escape,               "Warning: Invalid escape character"},
		{nepeta::parser_error::require_newline,              "Warning: Whitespace until newline is required after this point"}
	};
	
	std::cerr << "(line: " << line << ", column: " << column << ") ";
	try
	{
		// Find the error in the error-to-string mapping
		const std::string& ex = error_mapping.at(error);
		std::cerr << ex;
		
		// Output the character for errors that use it
		if(error == nepeta::parser_error::illegal_character || error == nepeta::parser_error::invalid_escape)
		{
			std::cerr << " '" << ch << "'";
		}
	}
	catch(const std::exception& e)
	{
		std::cerr << "Unknown error";
	}
	
	std::cerr << std::endl;
}

// Reads the entire file from 'path' and returns a nepeta document parsed from this file
nepeta::document read_file(const std::string& path)
{
	std::fstream fp(path);
	if(!fp)
	{
		throw std::runtime_error("Unable to open path: " + path);
	}
	
	// Read the entire file to a string
	auto contents = std::string(std::istreambuf_iterator<char>(fp), std::istreambuf_iterator<char>());
	
	// Load the string to a nepeta document, passing errors to the 'print_error' function
	return nepeta::load(contents, print_error);
}


int main(int argc, char* argv[])
{
	if(argc < 2)
	{
		std::cout << "Usage: read_file FILENAME" << std::endl;
		return 1;
	}
	
	// Read the file into a document
	const auto doc = read_file(argv[1]);
	
	// Dump the file back to the console, converted to a nepeta string
	nepeta::write_to_stream(std::cout, doc);
	
	return 0;
}
