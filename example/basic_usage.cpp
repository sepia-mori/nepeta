// Part of the nepeta project.
// Original source: https://git.sepiamori.com/sepia-mori/nepeta
//
//          Copyright Sepia Mori AS 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

// This example demonstrate various ways to use the library.
#include <iostream>
#include <cassert>

#include "nepeta.h"

const std::string_view test_document_source = R"NYA(
	MultipleNodes
	MultipleNodes
	MultipleNodes
	
	#NESTED
		NestedNode
		TargetNode
	#
	
	WithData {
		Text on line one
		Text on line two
	} second_data
	WithTypes true -50 123
)NYA";

void write_function_message(const std::string& msg)
{
	std::cout << "\n== " << msg << " ==" << std::endl;
}

// Demonstrates various ways to find nodes in documents
void find_node()
{
	write_function_message("find_node()");
	
	const nepeta::document doc = nepeta::load(test_document_source);
	
	{
		std::cout << "Finding single node:" << std::endl;
		const nepeta::document* node = nepeta::find(doc, "NESTED");
		assert(node && "No node found");
		std::cout << node->id << std::endl;
	}
	
	{
		std::cout << "\nTrying to find nonexistent node:" << std::endl;
		const nepeta::document* node = nepeta::find(doc, "InvalidNode");
		std::cout << node << std::endl;
	}
}

// Demonstrates various ways to iterate through document children and finding nodes matching a given ID
void find_all()
{
	write_function_message("find_all()");
	
	const nepeta::document doc = nepeta::load(test_document_source);
	
	std::cout << "Using direct access:" << std::endl;
	for(auto i = doc.children.begin(); i != doc.children.end(); ++i)
	{
		if(i->id == "MultipleNodes")
		{
			std::cout << i->id << std::endl;
		}
	}
	
	std::cout << "\nUsing nepeta::begin/end/next:" << std::endl;
	for(auto i = nepeta::begin(doc, "MultipleNodes"); i != nepeta::end(doc); i = nepeta::next(doc, i, "MultipleNodes"))
	{
		std::cout << i->id << std::endl;
	}
	
	std::cout << "\nUsing nepeta::for_each:" << std::endl;
	nepeta::for_each(doc, "MultipleNodes", [](const auto& node)
	{
		std::cout << node.id << std::endl;
	});
}

// Demonstrates various ways to access the data inside nodes
void data_access()
{
	write_function_message("data_access()");
	
	const nepeta::document doc = nepeta::load(test_document_source);
	
	{
		const nepeta::document* node = nepeta::find(doc, "WithData");
		
		std::cout << "Iterating through all data:" << std::endl;
		for(const auto& d : node->data)
		{
			std::cout << '"' << d << '"' << std::endl;
		}
		
		std::cout << "\nAccessing data by index:" << std::endl;
		std::cout << '"' << nepeta::opt_data(*node, 0).value_or("") << '"' << std::endl;
		std::cout << '"' << nepeta::opt_data(*node, 1).value_or("") << '"' << std::endl;
		std::cout << '"' << nepeta::opt_data(*node, 2).value_or("") << '"' << std::endl;
	}
	
	{
		const nepeta::document* node = nepeta::find(doc, "WithTypes");
		
		std::cout << "\nAccessing data by type:" << std::endl;
		const auto first = nepeta::doc_as_bool(*node, 0, false);
		std::cout << first << std::endl;
		
		const auto second = nepeta::doc_as_integer<int>(*node, 1, 0);
		std::cout << second << std::endl;
		
		const auto third = nepeta::doc_as_integer<int>(*node, 2, 0);
		std::cout << third << std::endl;
	}
}

// Demonstrates various ways to create documents from scratch.
// All methods create the same document structure.
void build_document()
{
	write_function_message("build_document()");
	
	// Creating a document using initializers
	{
		const nepeta::document doc
		{
			"", {},
			// Document children
			{
				{"EmptyNode"},
				{"NodeWithData", {"data1", "data2"}},
				{"NodeWithChildren", {},
					{
						{"ChildNode"}
					}
				}
			}
		};
		
		std::cout << "Output of initialized document:" << std::endl;
		nepeta::write_to_stream(std::cout, doc);
		std::cout << std::endl;
	}
	
	// Creating a document by merging documents
	{
		nepeta::document doc;
		
		nepeta::document empty_node {"EmptyNode"};
		nepeta::document node_with_data {"NodeWithData", {"data1", "data2"}};
		nepeta::document child_node {"ChildNode"};
		nepeta::document node_with_children {"NodeWithChildren"};
		
		node_with_children.children.push_back(std::move(child_node));
		doc.children.push_back(std::move(empty_node));
		doc.children.push_back(std::move(node_with_data));
		doc.children.push_back(std::move(node_with_children));
		
		std::cout << "\nOutput of merged documents:" << std::endl;
		nepeta::write_to_stream(std::cout, doc);
		std::cout << std::endl;
	}
	
	// Creating a document by reading string fragments
	{
		nepeta::document doc;
		nepeta::load_into(doc, "EmptyNode");
		nepeta::load_into(doc, "NodeWithData data1 data2");
		nepeta::load_into(doc, "#NodeWithChildren\nChildNode\n#");
		
		std::cout << "\nOutput of loaded string fragments:" << std::endl;
		nepeta::write_to_stream(std::cout, doc);
		std::cout << std::endl;
	}
}

// Reads a source text into a document_view
void read_document_view()
{
	write_function_message("read_document_view()");
	
	// document_view points directly into the memory of 'source',
	// and must therefore be available for as long as the document view is in scope.
	std::string source = std::string(test_document_source);
	const nepeta::document_view doc_view = nepeta::load_view(source);
	
	std::cout << "Output of document_view:" << std::endl;
	nepeta::write_to_stream(std::cout, doc_view);
	std::cout << std::endl;
}

// Demonstrates various ways to write a document
void write_document()
{
	write_function_message("write_document()");
	
	const nepeta::document doc = nepeta::load(test_document_source);
	
	// Writing to a string
	{
		std::string str = nepeta::write_to_string(doc);
		std::cout << "String output:\n" << str << std::endl;
	}
	
	// Writing to a stream
	{
		std::cout << "\nStream output:" << std::endl;
		nepeta::write_to_stream(std::cout, doc);
		std::cout << std::endl;
	}
}

int main(void)
{
	find_node();
	find_all();
	
	data_access();
	
	build_document();
	read_document_view();
	write_document();
	
	return 0;
}
