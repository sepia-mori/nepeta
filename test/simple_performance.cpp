// Part of the nepeta project.
// Original source: https://git.sepiamori.com/sepia-mori/nepeta
//
//          Copyright Sepia Mori AS 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
#include "nepeta.h"

#include <chrono>
#include <iostream>

template<typename T>
void run_test(const std::string& source)
{
	auto data_size_mb = source.size() / (1024.0*1024.0);
	
	constexpr int num_parses = 200;
	std::chrono::duration<double> seconds_sum = std::chrono::seconds(0);
	for(int i=0; i<num_parses; ++i)
	{
		std::string source_copy = source;
		const auto before = std::chrono::high_resolution_clock::now();
		if constexpr(std::is_same_v<T, nepeta::document>)
		{
			auto first = nepeta::load(source_copy, nepeta::detail::empty_error_handler);
		}
		else
		{
			auto first = nepeta::load_view(source_copy, nepeta::detail::empty_error_handler);
		}
		const auto after = std::chrono::high_resolution_clock::now();
		seconds_sum += after-before;
	}
	
	const auto avg = seconds_sum.count() / num_parses;
	std::cout << "Time taken: " << seconds_sum.count() << std::endl;
	std::cout << "MB size: " << data_size_mb << std::endl;
	std::cout << "MB/s: " << (data_size_mb / avg) << std::endl;
}

int main(void)
{
	std::string_view source_text = R"NYA(
#FIRST_NODE data1 data2 data3 "string 1\nstring 1\nstring 1" "string2\nstring2\nstring2" {
		Velit ratione accusamus ratione ut eos temporibus laudantium quas.
		Incidunt maxime itaque sapiente rerum.
		Deserunt eos est cupiditate ullam.
		Voluptates debitis dolor velit et alias.
}
	#NESTED data1 data2 data3 "string 1\nstring 1\nstring 1" "string2\nstring2\nstring2" {
		Velit ratione accusamus ratione ut eos temporibus laudantium quas.
		Incidunt maxime itaque sapiente rerum.
		Deserunt eos est cupiditate ullam.
		Voluptates debitis dolor velit et alias.
	}
		Node data1 data2 "data 3"
		Node data1 data2 "data 3"
		Node data1 data2 "data 3"
	#
	
	#NESTED_2 data1 data2 data3 "string 1\nstring 1\nstring 1" "string2\nstring2\nstring2" {
		Velit ratione accusamus ratione ut eos temporibus laudantium quas.
		Incidunt maxime itaque sapiente rerum.
		Deserunt eos est cupiditate ullam.
		Voluptates debitis dolor velit et alias.
	}
		Node data1 data2 "data 3"
		Node data1 data2 "data 3"
	#
#
)NYA";
	std::string source_reference;
	constexpr int num_copies = 2000;
	for(int i=0; i<num_copies; ++i)
	{
		source_reference += source_text;
	}
	
	run_test<nepeta::document>(source_reference);
	run_test<nepeta::document_view>(source_reference);
	
	return 0;
}
