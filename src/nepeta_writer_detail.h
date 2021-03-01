// Part of the nepeta project.
// Original source: https://git.sepiamori.com/sepia-mori/nepeta
//
//          Copyright Sepia Mori AS 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
#ifndef NEPETA_NEPETA_WRITER_DETAIL_H
#define NEPETA_NEPETA_WRITER_DETAIL_H

#include <algorithm>

#include "nepeta_traits.h"
#include "nepeta_base64.h"
#include "nepeta_writer.h"

namespace nepeta::detail
{

template<typename NepetaType, typename WriterType>
void nepeta_writer<NepetaType, WriterType>::write_node(const NepetaType& node, unsigned int current_depth) const
{
	if(current_depth > 0)
	{
		write_indentation(current_depth-1);
		if(!node.children.empty())
		{
			write(traits_type::c_nested_node_marker);
		}
		
		const auto type = determine_identifier_type(node.id);
		if(type == target_data_type::identifier)
		{
			write(node.id);
		}
		else
		{
			write_string(node.id);
		}
		
		for(const data_type& data : node.data)
		{
			write_data(data, current_depth-1);
		}
		
		write_newline();
	}
	
	for(const NepetaType& child : node.children)
	{
		write_node(child, current_depth+1);
	}
	
	if(current_depth > 0 && !node.children.empty())
	{
		write_indentation(current_depth-1);
		write(traits_type::c_nested_node_marker);
		write_newline();
	}
}

template<typename NepetaType, typename WriterType>
void nepeta_writer<NepetaType, WriterType>::write_data(const data_type& data, unsigned int current_depth) const
{
	write(traits_type::c_space);
	
	const auto type = determine_data_type(data);
	switch(type)
	{
		case target_data_type::identifier:
			write(data);
			break;
		
		case target_data_type::string:
			write_string(data);
			break;
			
		case target_data_type::block:
			write_block(data, current_depth);
			break;
			
		case target_data_type::block_base64:
			write_base64(data, current_depth);
			break;
	}
}

template<typename NepetaType, typename WriterType>
void nepeta_writer<NepetaType, WriterType>::write_base64(const data_type& data, unsigned int current_depth) const
{
	write(traits_type::c_block_data_open);
	write(traits_type::c_space);
	write(traits_type::base64_codec);
	write_newline();
	
	if(!data.empty())
	{
		write_indentation(current_depth+1);
		const size_type max_aligned_characters = data.size() - (data.size() % size_type{3});
		size_type characters_written = 0;
		for(size_type i=0; i<max_aligned_characters; i+=size_type{3})
		{
			typename NepetaType::char_type result[4] = {};
			detail::convert_to_base64_fragment<3>(result, data.data()+i);
			write(view_type(result, 4));
			characters_written += size_type{4};
			
			if(characters_written >= writer_param.base64_per_line && i+size_type{1} < max_aligned_characters)
			{
				write_newline();
				write_indentation(current_depth+1);
				characters_written = size_type{0};
			}
		}
		
		if(const auto remainder = data.size()%size_type{3}; remainder != size_type{0})
		{
			typename NepetaType::char_type result[4] = {};
			
			if(remainder == size_type{1})
			{
				detail::convert_to_base64_fragment<1>(result, data.data()+data.size()-remainder);
			}
			else if(remainder == size_type{2})
			{
				detail::convert_to_base64_fragment<2>(result, data.data()+data.size()-remainder);
			}
			
			write(view_type(result, 4));
		}
		
		write_newline();
	}
	
	write_indentation(current_depth);
	write(traits_type::c_block_data_close);
}

template<typename NepetaType, typename WriterType>
void nepeta_writer<NepetaType, WriterType>::write_block(const data_type& data, unsigned int current_depth) const
{
	write(traits_type::c_block_data_open);
	write_newline();
	
	if(!data.empty())
	{
		write_indentation(current_depth+1);
		for(typename data_type::size_type i=0; i<data.size(); ++i)
		{
			const auto& ch = data[i];
			if(i == 0 && traits_type::is_whitespace(ch))
			{
				write(traits_type::c_backslash);
				write(ch);
			}
			else if(traits_type::is_escape_character_marker(ch))
			{
				write(traits_type::c_backslash);
				write(traits_type::c_backslash);
			}
			else if(ch == traits_type::c_newline)
			{
				write_newline();
				write_indentation(current_depth+1);
			}
			else if(ch == traits_type::c_carriage_return)
			{
				write(traits_type::c_backslash);
				write(to_escaped_character(ch));
			}
			else
			{
				write(ch);
			}
		}
		write_newline();
	}
	
	write_indentation(current_depth);
	write(traits_type::c_block_data_close);
}

template<typename NepetaType, typename WriterType>
void nepeta_writer<NepetaType, WriterType>::write_string(const data_type& data) const
{
	write(traits_type::c_double_quote);
	for(typename data_type::size_type i=0; i<data.size(); ++i)
	{
		const auto& ch = data[i];
		if(traits_type::is_escape_character_marker(ch))
		{
			write(traits_type::c_backslash);
			write(traits_type::c_backslash);
		}
		else if(traits_type::is_string_marker(ch))
		{
			write(traits_type::c_backslash);
			write(traits_type::c_double_quote);
		}
		else if(traits_type::is_newline(ch))
		{
			write(traits_type::c_backslash);
			write(to_escaped_character(ch));
		}
		else
		{
			write(ch);
		}
	}
	write(traits_type::c_double_quote);
}

template<typename NepetaType, typename WriterType>
auto nepeta_writer<NepetaType, WriterType>::determine_identifier_type(const data_type& data) const -> target_data_type
{
	for(typename data_type::size_type i=0; i<data.size(); ++i)
	{
		const auto& ch = data[i];
		if(!traits_type::is_identifier(ch))
		{
			return target_data_type::string;
		}
	}
	
	return target_data_type::identifier;
}

template<typename NepetaType, typename WriterType>
auto nepeta_writer<NepetaType, WriterType>::determine_data_type(const data_type& data) const -> target_data_type
{
	using size_type = typename data_type::size_type;
	
	if(data.empty())
	{
		return target_data_type::string;
	}
	
	const auto max_check = std::min<decltype(data.size())>(
		data.size(),
		std::max(
			writer_param.limit_for_checking_binary,
			writer_param.limit_for_block_enforcement
		)
	);
	bool is_identifier = true;
	for(typename data_type::size_type i=0; i<max_check; ++i)
	{
		const auto& ch = data[i];
		if(!traits_type::is_identifier(ch))
		{
			is_identifier = false;
		}
		if(traits_type::is_binary(ch))
		{
			return target_data_type::block_base64;
		}
	}
	
	if(data.size() >= writer_param.limit_for_block_enforcement)
	{
		return target_data_type::block;
	}
	else if(is_identifier)
	{
		return target_data_type::identifier;
	}
	else
	{
		return target_data_type::string;
	}
}

template<typename NepetaType, typename WriterType>
auto nepeta_writer<NepetaType, WriterType>::to_escaped_character(char_type ch) const -> char_type
{
	switch(ch)
	{
		case traits_type::c_nullchar:			return traits_type::esc_nullchar;
		case traits_type::c_audible_bell:		return traits_type::esc_audible_bell;
		case traits_type::c_backspace:			return traits_type::esc_backspace;
		case traits_type::c_form_feed:			return traits_type::esc_form_feed;
		case traits_type::c_newline:			return traits_type::esc_newline;
		case traits_type::c_carriage_return:	return traits_type::esc_carriage_return;
		case traits_type::c_tab:				return traits_type::esc_tab;
		case traits_type::c_vertical_tab:		return traits_type::esc_vertical_tab;
		default: return ch;
	}
}

}

#endif
