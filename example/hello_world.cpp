// Part of the nepeta project.
// Original source: https://git.sepiamori.com/sepia-mori/nepeta
//
//          Copyright Sepia Mori AS 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

// This example demonstrates the absolute minimum core functionality pertaining to typical use.
#include <string>
#include <iostream>
#include <cassert>

#include "nepeta.h"

int main(int argc, char* argv[])
{
	// Parses a document from a string
	const nepeta::document document = nepeta::load("Hello world!");
	
	// Finds the node with the given ID
	const nepeta::document* hello = nepeta::find(document, "Hello");
	assert(hello && "Unable to find node");
	
	// Prints ID of 'hello', and the value of its first data entry
	std::cout << hello->id << ", " << nepeta::opt_data(*hello, 0).value_or("") << std::endl;
	
	return 0;
}
