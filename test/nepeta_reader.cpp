// Part of the nepeta project.
// Original source: https://git.sepiamori.com/sepia-mori/nepeta
//
//          Copyright Sepia Mori AS 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
#include "nepeta.h"
#include "test_helpers.h"

#include <boost/test/unit_test.hpp>

/**
 * Parsing of special cases
 */
BOOST_AUTO_TEST_SUITE(parsing_special_cases)
// @{

BOOST_AUTO_TEST_CASE_TEMPLATE(empty_source, T, nepeta_types)
{
	const auto td = test_data<T>::make
	(
		"", {}
	);
	
	BOOST_TEST( (td.view == td.reference) );
	BOOST_TEST( (td.error_v.empty()) );
}

BOOST_AUTO_TEST_CASE_TEMPLATE(newline_source, T, nepeta_types)
{
	const auto td = test_data<T>::make
	(
		"\n", {}
	);
	
	BOOST_TEST( (td.view == td.reference) );
	BOOST_TEST( (td.error_v.empty()) );
}

BOOST_AUTO_TEST_CASE_TEMPLATE(mixed_source_with_invalid, T, nepeta_types)
{
	const auto reference_id = std::string("\n\0\0", 3);
	const auto td = test_data<T>::make
	(
		reference_id, {}
	);
	
	BOOST_TEST( (td.view == td.reference) );
	BOOST_TEST( td.error_v == (std::vector<error_entry>{
		error_entry{parser_error::illegal_character, '\0', 2,1},
		error_entry{parser_error::illegal_character, '\0', 2,2}
	}) );
}

BOOST_AUTO_TEST_SUITE_END()
// @}

BOOST_AUTO_TEST_SUITE(basic_node_parsing)
// @{

BOOST_AUTO_TEST_CASE_TEMPLATE(single_id, T, nepeta_types)
{
	const auto td = test_data<T>::make
	(
		"Key", {
			T{"Key"}
		}
	);
	
	BOOST_TEST( (td.view == td.reference) );
	BOOST_TEST( (td.error_v.empty()) );
}

BOOST_AUTO_TEST_CASE_TEMPLATE(multiple_id, T, nepeta_types)
{
	const auto td = test_data<T>::make
	(
		"Key\nKey2", {
			T{"Key"}, T{"Key2"}
		}
	);
	
	BOOST_TEST( (td.view == td.reference) );
	BOOST_TEST( (td.error_v.empty()) );
}

BOOST_AUTO_TEST_CASE_TEMPLATE(multiple_id_with_error, T, nepeta_types)
{
	const auto td = test_data<T>::make
	(
		"Key\n\x01\nKey2\n\x01\x02\nKey3\x03", {
			T{"Key"}, T{"Key2"}, T{"Key3"}
		}
	);
	
	BOOST_TEST( (td.view == td.reference) );
	BOOST_TEST( td.error_v == (std::vector<error_entry>{
		error_entry{parser_error::illegal_character, '\x01', 2,1},
		error_entry{parser_error::illegal_character, '\x01', 4,1},
		error_entry{parser_error::illegal_character, '\x02', 4,2},
		error_entry{parser_error::illegal_character, '\x03', 5,5},
	}) );
}

BOOST_AUTO_TEST_CASE_TEMPLATE(multiple_id_with_comment, T, nepeta_types)
{
	const auto td = test_data<T>::make
	(
R"NYA(
Key
/* Comment */
Key2
// Comment2
Key3
)NYA", {
			T{"Key"}, T{"Key2"}, T{"Key3"}
		}
	);
	
	BOOST_TEST( (td.view == td.reference) );
	BOOST_TEST( (td.error_v.empty()) );
}

BOOST_AUTO_TEST_CASE_TEMPLATE(ids_with_special_characters, T, nepeta_types)
{
	const auto td = test_data<T>::make
	(
R"NYA(
キー
(Parenthesis)
'Odd'
)NYA", {
			T{"キー"}, T{"(Parenthesis)"}, T{"'Odd'"}
		}
	);
	
	BOOST_TEST( (td.view == td.reference) );
	BOOST_TEST( (td.error_v.empty()) );
}

BOOST_AUTO_TEST_CASE_TEMPLATE(stringed_ids, T, nepeta_types)
{
	const auto td = test_data<T>::make
	(
R"NYA(
"string id without data"
"string id with data" "data"
"escaped\ncharacter"
)NYA", {
			T{"string id without data"},
			T{"string id with data", {"data"}},
			T{"escaped\ncharacter"}
		}
	);
	
	BOOST_TEST( (td.view == td.reference) );
	BOOST_TEST( (td.error_v.empty()) );
}

BOOST_AUTO_TEST_CASE_TEMPLATE(blocks_invalid_as_id, T, nepeta_types)
{
	const auto td = test_data<T>::make
	(
R"NYA(
{
	block
} data
)NYA", {
			T{"block"},
			T{"data"}
		}
	);
	
	BOOST_TEST( (td.view == td.reference) );
	BOOST_TEST( td.error_v == (std::vector<error_entry>{
		error_entry{parser_error::illegal_character, '{', 2,1},
		error_entry{parser_error::illegal_character, '}', 4,1}
	}) );
}

BOOST_AUTO_TEST_SUITE_END()
// @}

BOOST_AUTO_TEST_SUITE(parsing_id_context)
// @{

BOOST_AUTO_TEST_CASE_TEMPLATE(simple_data, T, nepeta_types)
{
	const auto td = test_data<T>::make("Key data1 data2 data3 da\tta4\tdata5", {
			T{"Key", {"data1", "data2", "data3", "da", "ta4", "data5"}}
		}
	);
	
	BOOST_TEST( (td.view == td.reference) );
	BOOST_TEST( (td.error_v.empty()) );
}

BOOST_AUTO_TEST_CASE_TEMPLATE(stringed_data, T, nepeta_types)
{
	const auto td = test_data<T>::make("Key data1 \"data2\" \"\\\"data3\\\"\" \"da\\\"ta4\" \"da\\nta5\"", {
			T{"Key", {"data1", "data2", "\"data3\"", "da\"ta4", "da\nta5"}}
		}
	);
	
	BOOST_TEST( (td.view == td.reference) );
	BOOST_TEST( (td.error_v.empty()) );
}

BOOST_AUTO_TEST_CASE_TEMPLATE(stringed_data_error, T, nepeta_types)
{
	const auto td = test_data<T>::make("Key \"da\nta\"", {
			T{"Key", {"da"}},
			T{"ta", {""}}
		}
	);
	
	BOOST_TEST( (td.view == td.reference) );
	BOOST_TEST( td.error_v == (std::vector<error_entry>{
		error_entry{parser_error::string_not_closed, '\0', 1,5},
		error_entry{parser_error::string_not_closed, '\0', 2,3},
	}) );
}

BOOST_AUTO_TEST_CASE_TEMPLATE(empty_block_data, T, nepeta_types)
{
	const auto td = test_data<T>::make("Key {\n}", {
			T{"Key", {""}}
		}
	);
	
	BOOST_TEST( (td.view == td.reference) );
	BOOST_TEST( (td.error_v.empty()) );
}

BOOST_AUTO_TEST_CASE_TEMPLATE(block_data, T, nepeta_types)
{
	const auto td = test_data<T>::make(
R"NYA(
Key data1 {
	data2
}
Key2 {
    data3
}
	Key3 {
        Space indentation
        Second line
	}

Key4 {
    \ with space before
}
Key5 {
	
}
Key6 {
    
}
)NYA", {
			T{"Key", {"data1", "data2"}},
			T{"Key2", {"data3"}},
			T{"Key3", {"Space indentation\nSecond line"}},
			T{"Key4", {" with space before"}},
			T{"Key5", {""}},
			T{"Key6", {""}}
		}
	);
	
	BOOST_TEST( (td.view == td.reference) );
	BOOST_TEST( (td.error_v.empty()) );
}

BOOST_AUTO_TEST_CASE_TEMPLATE(block_closing_misalignment, T, nepeta_types)
{
	const auto td = test_data<T>::make("Key {\n    ", {
			T{"Key", {""}}
		}
	);
	
	BOOST_TEST( (td.view == td.reference) );
	BOOST_TEST( td.error_v == (std::vector<error_entry>{
		error_entry{parser_error::block_not_closed, '\0', 1,5}
	}) );
}

BOOST_AUTO_TEST_CASE_TEMPLATE(block_data_error, T, nepeta_types)
{
	const auto td = test_data<T>::make("Key {", {
			T{"Key", {""}}
		}
	);
	
	BOOST_TEST( (td.view == td.reference) );
	BOOST_TEST( td.error_v == (std::vector<error_entry>{
		error_entry{parser_error::block_not_closed, '\0', 1,5}
	}) );
}

BOOST_AUTO_TEST_CASE_TEMPLATE(block_data_error_2, T, nepeta_types)
{
	const auto td = test_data<T>::make("Key {\n    data", {
			T{"Key", {"data"}}
		}
	);
	
	BOOST_TEST( (td.view == td.reference) );
	BOOST_TEST( td.error_v == (std::vector<error_entry>{
		error_entry{parser_error::block_not_closed, '\0', 1,5}
	}) );
}

BOOST_AUTO_TEST_CASE_TEMPLATE(block_data_error_3, T, nepeta_types)
{
	const auto td = test_data<T>::make("Key {\n    data\\", {
			T{"Key", {"data"}}
		}
	);
	
	BOOST_TEST( (td.view == td.reference) );
	BOOST_TEST( td.error_v == (std::vector<error_entry>{
		error_entry{parser_error::invalid_escape, '\0', 2,10},
		error_entry{parser_error::block_not_closed, '\0', 1,5}
	}) );
}

BOOST_AUTO_TEST_CASE_TEMPLATE(block_misaligned_closing_mark, T, nepeta_types)
{
	const auto td = test_data<T>::make("Key {\n\tempty\n\t}\n}", {
			T{"Key", {"empty\n}"}}
		}
	);
	
	BOOST_TEST( (td.view == td.reference) );
	BOOST_TEST( td.error_v == (std::vector<error_entry>{
		error_entry{parser_error::bad_block_close, '\0', 3,2}
	}) );
}

BOOST_AUTO_TEST_CASE_TEMPLATE(block_with_single_newline, T, nepeta_types)
{
	const auto td = test_data<T>::make(R"NYA(
Key {
	
	
}
)NYA", {
			T{"Key", {"\n"}}
		}
	);
	
	BOOST_TEST( (td.view == td.reference) );
	BOOST_TEST( td.error_v.empty() );
}

BOOST_AUTO_TEST_SUITE_END()
// @}

BOOST_AUTO_TEST_SUITE(block_codec)
// @{

BOOST_AUTO_TEST_CASE_TEMPLATE(empty_base64, T, nepeta_types)
{
	const auto td = test_data<T>::make("Key {base64\n}", {
			T{"Key", {""}}
		}
	);
	
	BOOST_TEST( (td.view == td.reference) );
	BOOST_TEST( (td.error_v.empty()) );
}

BOOST_AUTO_TEST_CASE_TEMPLATE(base64_alternate_alignment, T, nepeta_types)
{
	const auto td = test_data<T>::make("Key { base64\n}", {
			T{"Key", {""}}
		}
	);
	
	BOOST_TEST( (td.view == td.reference) );
	BOOST_TEST( (td.error_v.empty()) );
}

BOOST_AUTO_TEST_CASE_TEMPLATE(base64_basic_data, T, nepeta_types)
{
	const auto td = test_data<T>::make(R"NYA(
Key { base64
	aGVsbG8=
}

)NYA", {
			T{"Key", {"hello"}}
		}
	);
	
	BOOST_TEST( (td.view == td.reference) );
	BOOST_TEST( (td.error_v.empty()) );
}

BOOST_AUTO_TEST_CASE_TEMPLATE(base64_no_padding, T, nepeta_types)
{
	const auto td = test_data<T>::make(R"NYA(
Key { base64
	aGVsbG8
}

)NYA", {
			T{"Key", {"hello"}}
		}
	);
	
	BOOST_TEST( (td.view == td.reference) );
	BOOST_TEST( (td.error_v.empty()) );
}

BOOST_AUTO_TEST_CASE_TEMPLATE(base64_bad_characters_handled, T, nepeta_types)
{
	const auto reference_data = std::string("\0\0\0hello", 8);
	const auto td = test_data<T>::make(R"NYA(
Key { base64
	\ 	& 
	aGVsbG8=
}

)NYA", {
			T{"Key", {reference_data}}
		}
	);
	
	BOOST_TEST( (td.view == td.reference) );
	BOOST_TEST( (td.error_v.empty()) );
}

BOOST_AUTO_TEST_CASE_TEMPLATE(base64_bad_number_of_characters, T, nepeta_types)
{
	// A bad base64 parser would fail with out-of-bounds if it tried to read blocks smaller than 4 characters
	const auto td = test_data<T>::make(R"NYA(
Key { base64
	a
}
Key { base64
	aG
}
Key { base64
	aGV
}
Key { base64
	aGVs
}
)NYA", {
			T{"Key", {""}},
			T{"Key", {"h"}},
			T{"Key", {"he"}},
			T{"Key", {"hel"}}
		}
	);
	
	BOOST_TEST( (td.view == td.reference) );
	BOOST_TEST( (td.error_v.empty()) );
}

BOOST_AUTO_TEST_CASE_TEMPLATE(base64_with_newline_ignored, T, nepeta_types)
{
	const auto td = test_data<T>::make(R"NYA(
Key { base64
	aGVs
	bG8=
}
)NYA", {T{"Key", {"hello"}}});
	
	BOOST_TEST( (td.view == td.reference) );
	BOOST_TEST( (td.error_v.empty()) );
}

BOOST_AUTO_TEST_CASE_TEMPLATE(block_bad_codec, T, nepeta_types)
{
	const auto td = test_data<T>::make("Key {badcodec\n}", {
			T{"Key", {""}}
		}
	);
	
	BOOST_TEST( (td.view == td.reference) );
	BOOST_TEST( td.error_v == (std::vector<error_entry>{
		error_entry{parser_error::bad_codec, '\0', 1,6}
	}) );
}

BOOST_AUTO_TEST_CASE_TEMPLATE(block_codec_invalid_character, T, nepeta_types)
{
	const auto td = test_data<T>::make("Key {base64 error\n}", {
			T{"Key", {""}}
		}
	);
	
	BOOST_TEST( (td.view == td.reference) );
	BOOST_TEST( td.error_v == (std::vector<error_entry>{
		error_entry{parser_error::require_newline, 'e', 1,13}
	}) );
}

BOOST_AUTO_TEST_SUITE_END()
// @}

BOOST_AUTO_TEST_SUITE(parsing_hash_context)
// @{

BOOST_AUTO_TEST_CASE_TEMPLATE(simple_hash, T, nepeta_types)
{
	const auto td = test_data<T>::make
	(
R"NYA(
#HASH
#
)NYA", {
			T{"HASH"}
		}
	);
	
	BOOST_TEST( (td.view == td.reference) );
	BOOST_TEST( (td.error_v.empty()) );
}

BOOST_AUTO_TEST_CASE_TEMPLATE(nested_hash, T, nepeta_types)
{
	const auto td = test_data<T>::make
	(
R"NYA(
#HASH
	#NESTED
		#THIRD
		#
		
		AnotherKey
	#
#
)NYA", {
			T{"HASH", {}, {
				T{"NESTED", {}, {
					T{"THIRD"},
					T{"AnotherKey"}
				}}
			}}
		}
	);
	
	BOOST_TEST( (td.view == td.reference) );
	BOOST_TEST( (td.error_v.empty()) );
}

BOOST_AUTO_TEST_CASE_TEMPLATE(bad_hash, T, nepeta_types)
{
	const auto td = test_data<T>::make
	(
R"NYA(
#HASH
)NYA", {
			T{"HASH"}
		}
	);
	
	BOOST_TEST( (td.view == td.reference) );
	BOOST_TEST( td.error_v == (std::vector<error_entry>{
		error_entry{parser_error::node_not_closed, '\0', 2,1}
	}) );
}

BOOST_AUTO_TEST_CASE_TEMPLATE(bad_empty_hash, T, nepeta_types)
{
	const auto td = test_data<T>::make("#", {});
	
	BOOST_TEST( (td.view == td.reference) );
	BOOST_TEST( td.error_v == (std::vector<error_entry>{
		error_entry{parser_error::too_may_node_closing_markers, '\0', 1,1}
	}) );
}

BOOST_AUTO_TEST_CASE_TEMPLATE(double_hash, T, nepeta_types)
{
	const auto td = test_data<T>::make("##test\nnested\n#", {
		T{"test", {}, {
			T{"nested"}
		}}
	});
	
	BOOST_TEST( (td.view == td.reference) );
	BOOST_TEST( td.error_v == (std::vector<error_entry>{
		error_entry{parser_error::illegal_character, '#', 1,2}
	}) );
}

BOOST_AUTO_TEST_SUITE_END()
// @}

BOOST_AUTO_TEST_SUITE(semicolon_as_newline)
// @{

BOOST_AUTO_TEST_CASE_TEMPLATE(proper_usage, T, nepeta_types)
{
	const auto td = test_data<T>::make
	(
R"NYA(
Key1 ; Key2
Key3; Key4
Key5 value1 "value2"
)NYA", {
			T{"Key1"}, T{"Key2"}, T{"Key3"}, T{"Key4"},
			T{"Key5", {"value1", "value2"}}
		}
	);
	
	BOOST_TEST( (td.view == td.reference) );
	BOOST_TEST( (td.error_v.empty()) );
}

BOOST_AUTO_TEST_CASE_TEMPLATE(proper_usage_with_nested, T, nepeta_types)
{
	const auto td = test_data<T>::make
	(
R"NYA(
#Key1; Key2
#

#Key3; Key4; #

#Key5 ; Key6 ; #
)NYA", {
			T{"Key1", {}, {T{"Key2"}}},
			T{"Key3", {}, {T{"Key4"}}},
			T{"Key5", {}, {T{"Key6"}}}
		}
	);
	
	BOOST_TEST( (td.view == td.reference) );
	BOOST_TEST( (td.error_v.empty()) );
}

BOOST_AUTO_TEST_SUITE_END()
// @}

BOOST_AUTO_TEST_SUITE(comment_parsing)
// @{

BOOST_AUTO_TEST_CASE_TEMPLATE(unclosed_multiline_comment, T, nepeta_types)
{
	const auto td = test_data<T>::make
	(
R"NYA(
/*
 * 
 * 
 *
)NYA", {});
	
	BOOST_TEST( (td.view == td.reference) );
	BOOST_TEST( td.error_v == (std::vector<error_entry>{
		error_entry{parser_error::comment_not_closed, '\0', 2,1}
	}) );
}

BOOST_AUTO_TEST_CASE_TEMPLATE(key_after_multiline, T, nepeta_types)
{
	const auto td = test_data<T>::make
	(
R"NYA(
/*
 * 
 */Key

/*
 *
 */ Key2
)NYA", {T{"Key"}, T{"Key2"}});
	
	BOOST_TEST( (td.view == td.reference) );
	BOOST_TEST( (td.error_v.empty()) );
}

BOOST_AUTO_TEST_CASE_TEMPLATE(key_after_single_line, T, nepeta_types)
{
	const auto td = test_data<T>::make
	(
R"NYA(
//
Key
)NYA", {T{"Key"}});
	
	BOOST_TEST( (td.view == td.reference) );
	BOOST_TEST( (td.error_v.empty()) );
}

BOOST_AUTO_TEST_CASE_TEMPLATE(invalid_multiline_comment, T, nepeta_types)
{
	const auto td = test_data<T>::make("/*/", {});
	
	BOOST_TEST( (td.view == td.reference) );
	BOOST_TEST( td.error_v == (std::vector<error_entry>{
		error_entry{parser_error::comment_not_closed, '\0', 1,1}
	}) );
}

BOOST_AUTO_TEST_CASE_TEMPLATE(node_data_comment, T, nepeta_types)
{
	const auto td = test_data<T>::make("Node /*/path/to/file*/data", {
		T{"Node", {"data"}}
	});
	
	BOOST_TEST( (td.view == td.reference) );
	BOOST_TEST( td.error_v.empty() );
}

BOOST_AUTO_TEST_CASE_TEMPLATE(ambiguous_comment, T, nepeta_types)
{
	const auto td = test_data<T>::make("Node /path/to/file", {
		T{"Node", {"/path/to/file"}}
	});
	
	BOOST_TEST( (td.view == td.reference) );
	BOOST_TEST( td.error_v.empty() );
}

BOOST_AUTO_TEST_SUITE_END()
// @}

BOOST_AUTO_TEST_SUITE(newline_format_handling)
// @{

BOOST_AUTO_TEST_CASE_TEMPLATE(crlf_newline, T, nepeta_types)
{
	const auto reference_id = std::string("\r\n\0", 3);
	const auto td = test_data<T>::make(reference_id, {});
	
	BOOST_TEST( (td.view == td.reference) );
	BOOST_TEST( td.error_v == (std::vector<error_entry>{
		error_entry{parser_error::illegal_character, '\0', 2,1},
	}) );
}

BOOST_AUTO_TEST_CASE_TEMPLATE(crlf_newline_block, T, nepeta_types)
{
	const auto td = test_data<T>::make
	(
"Key {\r\n"
"	data\r\n"
"	more data\r\n"
"}\r\n"
, {T{"Key", {"data\nmore data"}}});
	
	BOOST_TEST( (td.view == td.reference) );
	BOOST_TEST( td.error_v.empty() );
}

BOOST_AUTO_TEST_SUITE_END()
// @}

// Generally unicode works exactly as any other character, but we should try a few things just in case
BOOST_AUTO_TEST_SUITE(unicode_support)
// @{

BOOST_AUTO_TEST_CASE_TEMPLATE(basic_unicode, T, nepeta_types)
{
	const auto td = test_data<T>::make(R"NYA(
🍵 tea
tea 🍵
coffee "☕"
"☕" coffee
multiple_codes 🍵☕
block {
	🍵☕
}

)NYA", {
		T{"🍵", {"tea"}},
		T{"tea", {"🍵"}},
		T{"coffee", {"☕"}},
		T{"☕", {"coffee"}},
		T{"multiple_codes", {"🍵☕"}},
		T{"block", {"🍵☕"}},
	});
	
	BOOST_TEST( (td.view == td.reference) );
	BOOST_TEST( td.error_v.empty() );
}

BOOST_AUTO_TEST_CASE_TEMPLATE(unicode_before_eof, T, nepeta_types)
{
	const auto td = test_data<T>::make(R"NYA(🍵)NYA", {
		T{"🍵"}
	});
	
	BOOST_TEST( (td.view == td.reference) );
	BOOST_TEST( td.error_v.empty() );
}

BOOST_AUTO_TEST_SUITE_END()
// @}

// Test for very deep recursion
BOOST_AUTO_TEST_SUITE(crash_testing)
// @{

BOOST_AUTO_TEST_CASE_TEMPLATE(deep_recursion, T, nepeta_types)
{
	constexpr int num_nestings = 2000;
	
	std::string source;
	for(int i=0; i<num_nestings; ++i)
	{
		source.append("#Nested\n");
	}
	for(int i=0; i<num_nestings; ++i)
	{
		source.append("#\n");
	}
	
	const auto td = test_data<T>::make(source, {});
	
	const T* cur = &td.view;
	for(int i=0; i<num_nestings; ++i)
	{
		BOOST_REQUIRE(cur && !cur->children.empty());
		cur = &cur->children.front();
	}
	
	BOOST_TEST( td.error_v == (std::vector<error_entry>{
		error_entry{parser_error::recursion_limit_reached, '\0', 2000,1},
		error_entry{parser_error::too_may_node_closing_markers, '\0', 4000,1},
	}) );
}

BOOST_AUTO_TEST_SUITE_END()
// @}
