// Part of the nepeta project.
// Original source: https://git.sepiamori.com/sepia-mori/nepeta
//
//          Copyright Sepia Mori AS 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
#ifndef NEPETA_NEPETA_BASE64_H
#define NEPETA_NEPETA_BASE64_H

#include <cstdint>
#include <string_view>

#include "nepeta_traits.h"

namespace nepeta::detail
{

template<int InputSize, typename DataType>
void convert_to_base64_fragment(DataType* output, const DataType* input)
{
	constexpr auto table = traits<DataType>::base64_encoding_table;
	
	const auto get = [&input](auto idx) -> unsigned char { return static_cast<unsigned char>(input[idx]); };
	
	output[0] = table[( (get(0)       )>>2) & 0x3F];
	output[1] = table[( (get(0) & 0x03)<<4) | ((get(1)>>4) & 0x0F)];
	
	if constexpr(InputSize >= 2)
	{
		output[2] = table[( (get(1) & 0x0F)<<2) | (InputSize >= 3 ? ((get(2)>>6) & 0x03) : 0)];
	}
	if constexpr(InputSize >= 3)
	{
		output[3] = table[( (get(2) & 0x3F) )];
	}
	
	if constexpr(InputSize < 3)
	{
		output[3] = traits<DataType>::base64_padding_character;
	}
	if constexpr(InputSize < 2)
	{
		output[2] = traits<DataType>::base64_padding_character;
	}
}

// Taken and adapted from: https://stackoverflow.com/a/37109258
template<typename DataType, typename SizeType>
auto convert_from_base64(DataType* p_out, const SizeType size)
{
	constexpr std::uint_fast32_t base64_index[256] =
	{
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  62, 63, 62, 62, 63,
		52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 0,  0,  0,  0,  0,  0,
		0,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14,
		15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 0,  0,  0,  0,  63,
		0,  26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
		41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51
	};
	
	constexpr auto bytes_per_block = SizeType{4};
	
	const auto* p_in = p_out;
	
	const auto last_aligned = size - (size%bytes_per_block);
	auto write_i = SizeType{0};
	auto read_i = SizeType{0};
	
	for (; read_i < last_aligned; read_i += bytes_per_block)
	{
		const std::uint_fast32_t n =
			base64_index[static_cast<unsigned char>(p_in[read_i])] << 18 |
			base64_index[static_cast<unsigned char>(p_in[read_i + SizeType{1}])] << 12 |
			base64_index[static_cast<unsigned char>(p_in[read_i + SizeType{2}])] << 6 |
			base64_index[static_cast<unsigned char>(p_in[read_i + SizeType{3}])];
		p_out[write_i++] = static_cast<DataType>((n>>16) & 0xFF);
		p_out[write_i++] = static_cast<DataType>((n>>8 ) & 0xFF);
		p_out[write_i++] = static_cast<DataType>((n    ) & 0xFF);
	}
	
	if(size >= bytes_per_block)
	{
		write_i -= traits<DataType>::is_base64_padding_character(p_in[read_i-1]) ? 1 : 0;
		write_i -= traits<DataType>::is_base64_padding_character(p_in[read_i-2]) ? 1 : 0;
	}
	
	const auto bytes_remaining = size%bytes_per_block;
	if (bytes_remaining == SizeType{1})
	{
	}
	else if(bytes_remaining == SizeType{2})
	{
		const std::uint_fast32_t n =
			base64_index[static_cast<unsigned char>(p_in[read_i])] << 18 |
			base64_index[static_cast<unsigned char>(p_in[read_i + SizeType{1}])] << 12;
		p_out[write_i++] = static_cast<DataType>((n>>16) & 0xFF);
	}
	else if(bytes_remaining == SizeType{3})
	{
		const std::uint_fast32_t n =
			base64_index[static_cast<unsigned char>(p_in[read_i])] << 18 |
			base64_index[static_cast<unsigned char>(p_in[read_i + SizeType{1}])] << 12 |
			base64_index[static_cast<unsigned char>(p_in[read_i + SizeType{2}])] << 6;
		p_out[write_i++] = static_cast<DataType>((n>>16) & 0xFF);
		p_out[write_i++] = static_cast<DataType>((n>>8 ) & 0xFF);
	}
	
	return write_i;
}

}

#endif
