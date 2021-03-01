// Part of the nepeta project.
// Original source: https://git.sepiamori.com/sepia-mori/nepeta
//
//          Copyright Sepia Mori AS 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
#include "nepeta.h"
#include "test_helpers.h"

#include <cstdint>
#include <limits>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(moving_documents)
// @{

BOOST_AUTO_TEST_CASE_TEMPLATE(moving_documents, T, nepeta_types)
{
	std::string source_first = "First\nSecond";
	T doc = load_for<T>(source_first, nepeta::detail::empty_error_handler);
	
	const T* first_child = &doc.children.at(0);
	const T* second_child = &doc.children.at(1);
	
	T second_doc = std::move(doc);
	
	const T* second_child_first = &second_doc.children.at(0);
	const T* second_child_second = &second_doc.children.at(1);
	
	BOOST_TEST(first_child == second_child_first);
	BOOST_TEST(second_child == second_child_second);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(merging_documents, T, nepeta_types)
{
	std::string source_first = "#First\nDummy\n#";
	auto first = load_for<T>(source_first, nepeta::detail::empty_error_handler);
	std::string source_second = "#Second\nDummy\n#";
	auto second = load_for<T>(source_second, nepeta::detail::empty_error_handler);
	
	const T* first_data = first.children.at(0).children.data();
	const T* second_data = second.children.at(0).children.data();
	
	T merged;
	nepeta::merge(merged, first);
	nepeta::merge(merged, second);
	
	const T* merged_child_first_data = merged.children.at(0).children.data();
	const T* merged_child_second_data = merged.children.at(1).children.data();
	
	BOOST_TEST(!first.children.empty());
	BOOST_TEST(!second.children.empty());
	BOOST_TEST(first_data != merged_child_first_data);
	BOOST_TEST(second_data != merged_child_second_data);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(merge_moving_documents, T, nepeta_types)
{
	std::string source_first = "#First\nDummy\n#";
	auto first = load_for<T>(source_first, nepeta::detail::empty_error_handler);
	std::string source_second = "#Second\nDummy\n#";
	auto second = load_for<T>(source_second, nepeta::detail::empty_error_handler);
	
	const T* first_data = first.children.at(0).children.data();
	const T* second_data = second.children.at(0).children.data();
	
	T merged;
	nepeta::merge(merged, std::move(first));
	nepeta::merge(merged, std::move(second));
	
	const T* merged_child_first_data = merged.children.at(0).children.data();
	const T* merged_child_second_data = merged.children.at(1).children.data();
	
	BOOST_TEST(first.children.empty());
	BOOST_TEST(second.children.empty());
	BOOST_TEST(first_data == merged_child_first_data);
	BOOST_TEST(second_data == merged_child_second_data);
}

BOOST_AUTO_TEST_SUITE_END()
// @}

BOOST_AUTO_TEST_SUITE(iteration)
// @{

BOOST_AUTO_TEST_CASE_TEMPLATE(empty_iteration, T, nepeta_types)
{
	const T doc = {};
	
	BOOST_TEST( (nepeta::begin(doc, "Alpha") == nepeta::end(doc)) );
	BOOST_TEST( (nepeta::rbegin(doc, "Alpha") == nepeta::rend(doc)) );
}

BOOST_AUTO_TEST_CASE_TEMPLATE(basic_iteration, T, nepeta_types)
{
	std::string source = R"NYA(
		Beta b1
		Beta b2
		Alpha a1
		Beta b3
		Alpha a2
		Beta b4
		Beta b5
		Alpha a3
		Alpha a4
		Alpha a5
		Beta b6
		Alpha a6
		Beta b7
		Beta b8
	)NYA";
	const T doc = load_for<T>(source, nepeta::detail::empty_error_handler);
	
	const std::vector<std::string> alpha_order = {"a1", "a2", "a3", "a4", "a5", "a6"};
	const std::vector<std::string> beta_order = {"b1", "b2", "b3", "b4", "b5", "b6", "b7", "b8"};
	
	// Forward iteration
	{
		auto idx=0;
		for(auto i = nepeta::begin(doc, "Alpha"); i != nepeta::end(doc); i = nepeta::next(doc, i, "Alpha"))
		{
			BOOST_TEST( i->data.at(0) == alpha_order.at(idx) );
			idx++;
		}
		
		BOOST_TEST(idx == alpha_order.size());
	}
	
	{
		auto idx=0;
		nepeta::for_each(doc, "Alpha", [&](const auto& doc)
		{
			BOOST_TEST(doc.data.at(0) == alpha_order.at(idx) );
			idx++;
		});
		
		BOOST_TEST(idx == alpha_order.size());
	}
	
	{
		auto idx=0;
		for(auto i = nepeta::begin(doc, "Beta"); i != nepeta::end(doc); i = nepeta::next(doc, i, "Beta"))
		{
			BOOST_TEST( i->data.at(0) == beta_order.at(idx) );
			idx++;
		}
		
		BOOST_TEST(idx == beta_order.size());
	}
	
	{
		auto idx=0;
		nepeta::for_each(doc, "Beta", [&](const auto& doc)
		{
			BOOST_TEST(doc.data.at(0) == beta_order.at(idx) );
			idx++;
		});
		
		BOOST_TEST(idx == beta_order.size());
	}
	
	
	// Reverse iteration
	{
		auto idx=alpha_order.size();
		for(auto i = nepeta::rbegin(doc, "Alpha"); i != nepeta::rend(doc); i = nepeta::rnext(doc, i, "Alpha"))
		{
			idx--;
			BOOST_TEST( i->data.at(0) == alpha_order.at(idx) );
		}
		
		BOOST_TEST(idx == 0);
	}
	
	{
		auto idx=alpha_order.size();
		nepeta::rfor_each(doc, "Alpha", [&](const auto& doc)
		{
			idx--;
			BOOST_TEST(doc.data.at(0) == alpha_order.at(idx) );
		});
		
		BOOST_TEST(idx == 0);
	}
	
	{
		auto idx=beta_order.size();
		for(auto i = nepeta::rbegin(doc, "Beta"); i != nepeta::rend(doc); i = nepeta::rnext(doc, i, "Beta"))
		{
			idx--;
			BOOST_TEST( i->data.at(0) == beta_order.at(idx) );
		}
		
		BOOST_TEST(idx == 0);
	}
	
	{
		auto idx=beta_order.size();
		nepeta::rfor_each(doc, "Beta", [&](const auto& doc)
		{
			idx--;
			BOOST_TEST(doc.data.at(0) == beta_order.at(idx) );
		});
		
		BOOST_TEST(idx == 0);
	}
}

BOOST_AUTO_TEST_SUITE_END()
// @}

BOOST_AUTO_TEST_SUITE(number_conversion)
// @{

BOOST_AUTO_TEST_CASE_TEMPLATE(bool_conversion, T, nepeta_types)
{
	const T doc = {"", {"true", "false", "", "invalid"}};
	
	BOOST_REQUIRE(  nepeta::doc_opt_bool(doc, 0).has_value() );
	BOOST_REQUIRE(  nepeta::doc_opt_bool(doc, 1).has_value() );
	BOOST_TEST( !nepeta::doc_opt_bool(doc, 2).has_value() );
	BOOST_TEST( !nepeta::doc_opt_bool(doc, 3).has_value() );
	BOOST_TEST( nepeta::doc_opt_bool(doc, 0).value() == true );
	BOOST_TEST( nepeta::doc_opt_bool(doc, 1).value() == false );
}

BOOST_AUTO_TEST_CASE_TEMPLATE(integer_conversion, T, nepeta_types)
{
	// Common
	{
		const T doc = {"", {"", "-", "+", "error"}};
		
		BOOST_REQUIRE(nepeta::doc_opt_integer<int>(doc, 0).has_value() );
		BOOST_REQUIRE(nepeta::doc_opt_integer<int>(doc, 1).has_value() );
		BOOST_REQUIRE(nepeta::doc_opt_integer<int>(doc, 2).has_value() );
		BOOST_TEST(!nepeta::doc_opt_integer<int>(doc, 3).has_value() );
		
		BOOST_TEST(nepeta::doc_as_integer<int>(doc, 0) == 0);
		BOOST_TEST(nepeta::doc_as_integer<int>(doc, 1) == 0);
		BOOST_TEST(nepeta::doc_as_integer<int>(doc, 2) == 0);
	}
	
	// int8_t
	{
		const T doc = {"", {"+10", "1'0", "10", "-10", "127", "-128"}};
		
		BOOST_REQUIRE(nepeta::doc_opt_integer<int8_t>(doc, 0).has_value() );
		BOOST_REQUIRE(nepeta::doc_opt_integer<int8_t>(doc, 1).has_value() );
		BOOST_REQUIRE(nepeta::doc_opt_integer<int8_t>(doc, 2).has_value() );
		BOOST_REQUIRE(nepeta::doc_opt_integer<int8_t>(doc, 3).has_value() );
		BOOST_REQUIRE(nepeta::doc_opt_integer<int8_t>(doc, 4).has_value() );
		BOOST_REQUIRE(nepeta::doc_opt_integer<int8_t>(doc, 5).has_value() );
		
		BOOST_TEST(nepeta::doc_as_integer<int8_t>(doc, 0) == int8_t{10});
		BOOST_TEST(nepeta::doc_as_integer<int8_t>(doc, 1) == int8_t{10});
		BOOST_TEST(nepeta::doc_as_integer<int8_t>(doc, 2) == int8_t{10});
		BOOST_TEST(nepeta::doc_as_integer<int8_t>(doc, 3) == int8_t{-10});
		BOOST_TEST(nepeta::doc_as_integer<int8_t>(doc, 4) == int8_t{127});
		BOOST_TEST(nepeta::doc_as_integer<int8_t>(doc, 5) == int8_t{-128});
	}
	
	// uint8_t
	{
		const T doc = {"", {"+10", "1'0", "10", "-0", "0", "255"}};
		
		BOOST_REQUIRE(nepeta::doc_opt_integer<uint8_t>(doc, 0).has_value() );
		BOOST_REQUIRE(nepeta::doc_opt_integer<uint8_t>(doc, 1).has_value() );
		BOOST_REQUIRE(nepeta::doc_opt_integer<uint8_t>(doc, 2).has_value() );
		BOOST_REQUIRE(nepeta::doc_opt_integer<uint8_t>(doc, 3).has_value() );
		BOOST_REQUIRE(nepeta::doc_opt_integer<uint8_t>(doc, 4).has_value() );
		BOOST_REQUIRE(nepeta::doc_opt_integer<uint8_t>(doc, 5).has_value() );
		
		BOOST_TEST(nepeta::doc_as_integer<uint8_t>(doc, 0) == uint8_t{10});
		BOOST_TEST(nepeta::doc_as_integer<uint8_t>(doc, 1) == uint8_t{10});
		BOOST_TEST(nepeta::doc_as_integer<uint8_t>(doc, 2) == uint8_t{10});
		BOOST_TEST(nepeta::doc_as_integer<uint8_t>(doc, 3) == uint8_t{0});
		BOOST_TEST(nepeta::doc_as_integer<uint8_t>(doc, 4) == uint8_t{0});
		BOOST_TEST(nepeta::doc_as_integer<uint8_t>(doc, 5) == uint8_t{255});
	}
	
	// int16_t
	{
		const T doc = {"", {"+10", "1'0", "10", "-10", "32767", "-32768"}};
		
		BOOST_REQUIRE(nepeta::doc_opt_integer<int16_t>(doc, 0).has_value() );
		BOOST_REQUIRE(nepeta::doc_opt_integer<int16_t>(doc, 1).has_value() );
		BOOST_REQUIRE(nepeta::doc_opt_integer<int16_t>(doc, 2).has_value() );
		BOOST_REQUIRE(nepeta::doc_opt_integer<int16_t>(doc, 3).has_value() );
		BOOST_REQUIRE(nepeta::doc_opt_integer<int16_t>(doc, 4).has_value() );
		BOOST_REQUIRE(nepeta::doc_opt_integer<int16_t>(doc, 5).has_value() );
		
		BOOST_TEST(nepeta::doc_as_integer<int16_t>(doc, 0) == int16_t{10});
		BOOST_TEST(nepeta::doc_as_integer<int16_t>(doc, 1) == int16_t{10});
		BOOST_TEST(nepeta::doc_as_integer<int16_t>(doc, 2) == int16_t{10});
		BOOST_TEST(nepeta::doc_as_integer<int16_t>(doc, 3) == int16_t{-10});
		BOOST_TEST(nepeta::doc_as_integer<int16_t>(doc, 4) == int16_t{32767});
		BOOST_TEST(nepeta::doc_as_integer<int16_t>(doc, 5) == int16_t{-32768});
	}
	
	// uint16_t
	{
		const T doc = {"", {"+10", "1'0", "10", "-0", "0", "65535"}};
		
		BOOST_REQUIRE(nepeta::doc_opt_integer<uint16_t>(doc, 0).has_value() );
		BOOST_REQUIRE(nepeta::doc_opt_integer<uint16_t>(doc, 1).has_value() );
		BOOST_REQUIRE(nepeta::doc_opt_integer<uint16_t>(doc, 2).has_value() );
		BOOST_REQUIRE(nepeta::doc_opt_integer<uint16_t>(doc, 3).has_value() );
		BOOST_REQUIRE(nepeta::doc_opt_integer<uint16_t>(doc, 4).has_value() );
		BOOST_REQUIRE(nepeta::doc_opt_integer<uint16_t>(doc, 5).has_value() );
		
		BOOST_TEST(nepeta::doc_as_integer<uint16_t>(doc, 0) == uint16_t{10});
		BOOST_TEST(nepeta::doc_as_integer<uint16_t>(doc, 1) == uint16_t{10});
		BOOST_TEST(nepeta::doc_as_integer<uint16_t>(doc, 2) == uint16_t{10});
		BOOST_TEST(nepeta::doc_as_integer<uint16_t>(doc, 3) == uint16_t{0});
		BOOST_TEST(nepeta::doc_as_integer<uint16_t>(doc, 4) == uint16_t{0});
		BOOST_TEST(nepeta::doc_as_integer<uint16_t>(doc, 5) == uint16_t{65535});
	}
	
	// int32_t
	{
		const T doc = {"", {"+10", "1'0", "10", "-10", "-2147483648", "2147483647"}};
		
		BOOST_REQUIRE(nepeta::doc_opt_integer<int32_t>(doc, 0).has_value() );
		BOOST_REQUIRE(nepeta::doc_opt_integer<int32_t>(doc, 1).has_value() );
		BOOST_REQUIRE(nepeta::doc_opt_integer<int32_t>(doc, 2).has_value() );
		BOOST_REQUIRE(nepeta::doc_opt_integer<int32_t>(doc, 3).has_value() );
		BOOST_REQUIRE(nepeta::doc_opt_integer<int32_t>(doc, 4).has_value() );
		BOOST_REQUIRE(nepeta::doc_opt_integer<int32_t>(doc, 5).has_value() );
		
		BOOST_TEST(nepeta::doc_as_integer<int32_t>(doc, 0) == int32_t{10});
		BOOST_TEST(nepeta::doc_as_integer<int32_t>(doc, 1) == int32_t{10});
		BOOST_TEST(nepeta::doc_as_integer<int32_t>(doc, 2) == int32_t{10});
		BOOST_TEST(nepeta::doc_as_integer<int32_t>(doc, 3) == int32_t{-10});
		BOOST_TEST(nepeta::doc_as_integer<int32_t>(doc, 4) == int32_t{-2147483648});
		BOOST_TEST(nepeta::doc_as_integer<int32_t>(doc, 5) == int32_t{2147483647});
	}
	
	// uint32_t
	{
		const T doc = {"", {"+10", "1'0", "10", "-0", "0", "4294967295"}};
		
		BOOST_REQUIRE(nepeta::doc_opt_integer<uint32_t>(doc, 0).has_value() );
		BOOST_REQUIRE(nepeta::doc_opt_integer<uint32_t>(doc, 1).has_value() );
		BOOST_REQUIRE(nepeta::doc_opt_integer<uint32_t>(doc, 2).has_value() );
		BOOST_REQUIRE(nepeta::doc_opt_integer<uint32_t>(doc, 3).has_value() );
		BOOST_REQUIRE(nepeta::doc_opt_integer<uint32_t>(doc, 4).has_value() );
		BOOST_REQUIRE(nepeta::doc_opt_integer<uint32_t>(doc, 5).has_value() );
		
		BOOST_TEST(nepeta::doc_as_integer<uint32_t>(doc, 0) == uint32_t{10});
		BOOST_TEST(nepeta::doc_as_integer<uint32_t>(doc, 1) == uint32_t{10});
		BOOST_TEST(nepeta::doc_as_integer<uint32_t>(doc, 2) == uint32_t{10});
		BOOST_TEST(nepeta::doc_as_integer<uint32_t>(doc, 3) == uint32_t{0});
		BOOST_TEST(nepeta::doc_as_integer<uint32_t>(doc, 4) == uint32_t{0});
		BOOST_TEST(nepeta::doc_as_integer<uint32_t>(doc, 5) == uint32_t{4294967295});
	}
	
	// int64_t
	{
		const T doc = {"", {"+10", "1'0", "10", "-10", "-9223372036854775807", "9223372036854775807"}};
		
		BOOST_REQUIRE(nepeta::doc_opt_integer<int64_t>(doc, 0).has_value() );
		BOOST_REQUIRE(nepeta::doc_opt_integer<int64_t>(doc, 1).has_value() );
		BOOST_REQUIRE(nepeta::doc_opt_integer<int64_t>(doc, 2).has_value() );
		BOOST_REQUIRE(nepeta::doc_opt_integer<int64_t>(doc, 3).has_value() );
		BOOST_REQUIRE(nepeta::doc_opt_integer<int64_t>(doc, 4).has_value() );
		BOOST_REQUIRE(nepeta::doc_opt_integer<int64_t>(doc, 5).has_value() );
		
		BOOST_TEST(nepeta::doc_as_integer<int64_t>(doc, 0) == int64_t{10});
		BOOST_TEST(nepeta::doc_as_integer<int64_t>(doc, 1) == int64_t{10});
		BOOST_TEST(nepeta::doc_as_integer<int64_t>(doc, 2) == int64_t{10});
		BOOST_TEST(nepeta::doc_as_integer<int64_t>(doc, 3) == int64_t{-10});
		BOOST_TEST(nepeta::doc_as_integer<int64_t>(doc, 4) == int64_t{-9223372036854775807});
		BOOST_TEST(nepeta::doc_as_integer<int64_t>(doc, 5) == int64_t{9223372036854775807});
	}
	
	// uint64_t
	{
		const T doc = {"", {"+10", "1'0", "10", "-0", "0", "18446744073709551615"}};
		
		BOOST_REQUIRE(nepeta::doc_opt_integer<uint64_t>(doc, 0).has_value() );
		BOOST_REQUIRE(nepeta::doc_opt_integer<uint64_t>(doc, 1).has_value() );
		BOOST_REQUIRE(nepeta::doc_opt_integer<uint64_t>(doc, 2).has_value() );
		BOOST_REQUIRE(nepeta::doc_opt_integer<uint64_t>(doc, 3).has_value() );
		BOOST_REQUIRE(nepeta::doc_opt_integer<uint64_t>(doc, 4).has_value() );
		BOOST_REQUIRE(nepeta::doc_opt_integer<uint64_t>(doc, 5).has_value() );
		
		BOOST_TEST(nepeta::doc_as_integer<uint64_t>(doc, 0) == uint64_t{10});
		BOOST_TEST(nepeta::doc_as_integer<uint64_t>(doc, 1) == uint64_t{10});
		BOOST_TEST(nepeta::doc_as_integer<uint64_t>(doc, 2) == uint64_t{10});
		BOOST_TEST(nepeta::doc_as_integer<uint64_t>(doc, 3) == uint64_t{0});
		BOOST_TEST(nepeta::doc_as_integer<uint64_t>(doc, 4) == uint64_t{0});
		BOOST_TEST(nepeta::doc_as_integer<uint64_t>(doc, 5) == uint64_t{18446744073709551615U});
	}
}

BOOST_AUTO_TEST_SUITE_END()
// @}
