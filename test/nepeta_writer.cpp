// Part of the nepeta project.
// Original source: https://git.sepiamori.com/sepia-mori/nepeta
//
//          Copyright Sepia Mori AS 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
#include <boost/test/unit_test.hpp>
#include <boost/mpl/list.hpp>

#include "nepeta.h"

BOOST_AUTO_TEST_SUITE(writer_special_cases)
// @{

BOOST_AUTO_TEST_CASE(empty_document)
{
	constexpr std::string_view reference_text = "";

	const nepeta::document n = nepeta::load(reference_text);
	const std::string result = nepeta::write_to_string(n);
	BOOST_TEST( result == "" );
}

BOOST_AUTO_TEST_CASE(invalid_root)
{
	const auto n = nepeta::document{
		"InvalidIdForRootNode",
		{"InvalidDataForRootNode"},
		{{"Node"}}
	};
	const std::string result = nepeta::write_to_string(n);
	BOOST_TEST( result == "Node\n" );
}

BOOST_AUTO_TEST_SUITE_END()
// @}

BOOST_AUTO_TEST_SUITE(basic_writer)
// @{

BOOST_AUTO_TEST_CASE(safe_binary_options)
{
	nepeta::writer_parameters params;
	
	const auto n = nepeta::document{
		"", {},
		{
			{"Node", {"\x01\x02\x03\x04\x05"}}
		}
	};
	
	{
		params.limit_for_checking_binary = 0;
		params.limit_for_block_enforcement = 0;
		const std::string result = nepeta::write_to_string(n, params);
		BOOST_TEST( result == "Node {\n\t\x01\x02\x03\x04\x05\n}\n" );
		BOOST_TEST( (nepeta::load(result) == n) );
	}
	
	{
		params.limit_for_checking_binary = 0;
		params.limit_for_block_enforcement = 100;
		const std::string result = nepeta::write_to_string(n, params);
		BOOST_TEST( result == "Node { base64\n\tAQIDBAU=\n}\n" );
		BOOST_TEST( (nepeta::load(result) == n) );
	}
}

BOOST_AUTO_TEST_CASE(string_identifier_handling)
{
	const auto n = nepeta::document{
		"", {},
		{
			{"String type identifier"}
		}
	};
	
	const std::string result = nepeta::write_to_string(n);
	BOOST_TEST( result == "\"String type identifier\"\n");
	BOOST_TEST( (nepeta::load(result) == n) );
}

BOOST_AUTO_TEST_CASE(indentation_options)
{
	const auto n = nepeta::document{
		"", {},
		{
			{"Node", {"text"},
				{
					{"Nested", {"more data"}}
				}
			}
		}
	};
	
	nepeta::writer_parameters params;
	params.indentation = nepeta::writer_parameters::indentation_type::spaces;
	params.indentation_characters = 4;
	const std::string result = nepeta::write_to_string(n, params);
	BOOST_TEST( result == "#Node text\n    Nested \"more data\"\n#\n" );
	BOOST_TEST( (nepeta::load(result) == n) );
}

BOOST_AUTO_TEST_CASE(base64_options)
{
	const auto n = nepeta::document{
		"", {},
		{{"Node", {std::string(10, '\x01')}}}
	};
	
	nepeta::writer_parameters params;
	{
		params.base64_per_line = 8;
		const std::string result = nepeta::write_to_string(n, params);
		BOOST_TEST( result == "Node { base64\n\tAQEBAQEB\n\tAQEBAQ==\n}\n" );
		BOOST_TEST( (nepeta::load(result) == n) );
	}
	
	{
		// Rounded up to the nearest 4 automatically
		params.base64_per_line = 7;
		const std::string result = nepeta::write_to_string(n, params);
		BOOST_TEST( result == "Node { base64\n\tAQEBAQEB\n\tAQEBAQ==\n}\n" );
	}
	
	{
		// Make sure zero is rounded up to 4
		params.base64_per_line = 0;
		const std::string result = nepeta::write_to_string(n, params);
		BOOST_TEST( result == "Node { base64\n\tAQEB\n\tAQEB\n\tAQEB\n\tAQ==\n}\n" );
	}
}

// Writing of all valid normal characters to an identifier
BOOST_AUTO_TEST_CASE(valid_normal_characters_identifier)
{
	auto doc = nepeta::document{
		"", {},
		{{"Node", {""}}}
	};
	
	auto& node = doc.children.at(0);
	for(unsigned char ch = '!'; true; ++ch)
	{
		if(
			ch != '\"' && ch != '#' && ch != ';' &&
			ch != '{'  && ch != '}' && ch != '\\'
		)
		{
			node.data[0].push_back(ch);
		}
		
		if(ch == 255)
		{
			break;
		}
	}
	
	nepeta::writer_parameters params;
	params.limit_for_block_enforcement = 1000;
	const std::string result = nepeta::write_to_string(doc, params);
	
	BOOST_TEST( result == "Node !$%&'()*+,-./0123456789:<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[]^_`abcdefghijklmnopqrstuvwxyz|~\x80\x81\x82\x83\x84\x85\x86\x87\x88\x89\x8A\x8B\x8C\x8D\x8E\x8F\x90\x91\x92\x93\x94\x95\x96\x97\x98\x99\x9A\x9B\x9C\x9D\x9E\x9F\xA0\xA1\xA2\xA3\xA4\xA5\xA6\xA7\xA8\xA9\xAA\xAB\xAC\xAD\xAE\xAF\xB0\xB1\xB2\xB3\xB4\xB5\xB6\xB7\xB8\xB9\xBA\xBB\xBC\xBD\xBE\xBF\xC0\xC1\xC2\xC3\xC4\xC5\xC6\xC7\xC8\xC9\xCA\xCB\xCC\xCD\xCE\xCF\xD0\xD1\xD2\xD3\xD4\xD5\xD6\xD7\xD8\xD9\xDA\xDB\xDC\xDD\xDE\xDF\xE0\xE1\xE2\xE3\xE4\xE5\xE6\xE7\xE8\xE9\xEA\xEB\xEC\xED\xEE\xEF\xF0\xF1\xF2\xF3\xF4\xF5\xF6\xF7\xF8\xF9\xFA\xFB\xFC\xFD\xFE\xFF\n" );
}

// Writing of all valid normal characters to a string
BOOST_AUTO_TEST_CASE(valid_normal_characters_string)
{
	auto doc = nepeta::document{
		"", {},
		{{"Node", {""}}}
	};
	
	auto& node = doc.children.at(0);
	for(unsigned char ch = ' '; true; ++ch)
	{
		node.data[0].push_back(ch);
		
		if(ch == 255)
		{
			break;
		}
	}
	
	nepeta::writer_parameters params;
	params.limit_for_block_enforcement = 1000;
	const std::string result = nepeta::write_to_string(doc, params);
	
	BOOST_TEST( result == "Node \" !\\\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\x80\x81\x82\x83\x84\x85\x86\x87\x88\x89\x8A\x8B\x8C\x8D\x8E\x8F\x90\x91\x92\x93\x94\x95\x96\x97\x98\x99\x9A\x9B\x9C\x9D\x9E\x9F\xA0\xA1\xA2\xA3\xA4\xA5\xA6\xA7\xA8\xA9\xAA\xAB\xAC\xAD\xAE\xAF\xB0\xB1\xB2\xB3\xB4\xB5\xB6\xB7\xB8\xB9\xBA\xBB\xBC\xBD\xBE\xBF\xC0\xC1\xC2\xC3\xC4\xC5\xC6\xC7\xC8\xC9\xCA\xCB\xCC\xCD\xCE\xCF\xD0\xD1\xD2\xD3\xD4\xD5\xD6\xD7\xD8\xD9\xDA\xDB\xDC\xDD\xDE\xDF\xE0\xE1\xE2\xE3\xE4\xE5\xE6\xE7\xE8\xE9\xEA\xEB\xEC\xED\xEE\xEF\xF0\xF1\xF2\xF3\xF4\xF5\xF6\xF7\xF8\xF9\xFA\xFB\xFC\xFD\xFE\xFF\"\n" );
}

// Testing a larger source, and checks that writing it back produces the
// same file.
BOOST_AUTO_TEST_CASE(read_write_equivalence)
{
	constexpr std::string_view reference_text =
R"NYA(SimpleData value1 value2
PlainNode
UnicodeData 🍵☕ 🍵 ☕
#NestedNode
	Key3 v1 v2
	Key4 v3 v4
#
#StringData "multiple words" "another	string" "escaped\nchar\r\n"
	Key6 {
		Large block of data Large block of data Large block of data
		Large block of data Large block of data Large block of data
		Large block of data Large block of data Large block of data
		Large block of data Large block of data Large block of data
		Large block of data Large block of data Large block of data
		Large block of data Large block of data Large block of data
	}
	Key7 { base64
		AQIDBAo=
	}
	EmptyString ""
	WhitespaceBlock {
		\ Large block of data Large block of data Large block of data
		Large block of data Large block of data Large block of data
		Large block of data Large block of data Large block of data
		Large block of data Large block of data Large block of data
		Large block of data Large block of data Large block of data
	}
	WhitespaceEscapedBlock {
		Large\rblock of data Large block of data Large block of data
		Large\rblock of data Large block of data Large block of data
		Large\rblock of data Large block of data Large block of data
		Large\rblock of data Large block of data Large block of data
		Large\rblock of data Large block of data Large block of data
	}
#
)NYA";

	const nepeta::document n = nepeta::load(reference_text);
	
	const std::string result = nepeta::write_to_string(n);
	BOOST_TEST( result == reference_text );
	BOOST_TEST( (nepeta::load(result) == n) );
}

BOOST_AUTO_TEST_SUITE_END()
// @}
