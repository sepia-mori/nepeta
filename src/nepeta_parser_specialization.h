// Part of the nepeta project.
// Original source: https://git.sepiamori.com/sepia-mori/nepeta
//
//          Copyright Sepia Mori AS 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
#ifndef NEPETA_NEPETA_PARSER_SPECIALIZATION_H
#define NEPETA_NEPETA_PARSER_SPECIALIZATION_H

#include <string>
#include <string_view>

#include "nepeta.h"
#include "nepeta_traits.h"

namespace nepeta
{
/// The default recursion limit for parsing nested nodes.
template<typename T>
constexpr T default_recursion_limit = 2'000;

/// The default maximum number of errors to raise before ignoring subsequent errors
template<typename T>
constexpr T default_maximum_error_limit = 10;
}

namespace nepeta::detail
{

template<typename TraitsType, typename BufferType, typename SizeType>
void line_column_from_buffer_position(BufferType&& buf, SizeType bufpos, SizeType& line, SizeType& column)
{
	line = 1;
	column = 1;
	
	// Calculate line-column from position
	for(SizeType i=0; i<bufpos; ++i)
	{
		if(buf[i] == TraitsType::c_newline)
		{
			line++;
			column = 1;
		}
		else
		{
			column++;
		}
	}
}


template<typename DocumentType, typename ErrorFunc>
struct parser_document
{
	using document_type = DocumentType;
	using data_type = typename document_type::data_type;
	using view_type = typename document_type::view_type;
	using size_type = typename data_type::size_type;
	using char_type = typename document_type::char_type;
	using traits_type = traits<char_type>;
	using buffer_type = view_type;
	
	data_type make_data()
	{
		return data_type();
	}
	
	void add_character(data_type& dst, char_type ch)
	{
		dst.push_back(ch);
	}
	void copy_string(data_type& dst, view_type src)
	{
		dst.append(src.data(), src.size());
	}
	void resize_string(data_type& dst, size_type n_bytes)
	{
		dst.resize(n_bytes);
	}
	char_type* data_of(data_type& src)
	{
		return src.data();
	}
	
	void next_character()
	{
		++buffer_position;
	}
	char_type current_character() const
	{
		return source[buffer_position];
	}
	char_type peek_next_character() const
	{
		return next_eof() ? traits_type::c_nullchar : source[buffer_position+1];
	}
	bool eof() const { return buffer_position >= source.size(); }
	bool next_eof() const { return buffer_position >= source.size()-1; }
	
	void error(parser_error type, char_type ch, size_type buffer_position)
	{
		if(error_limit == 0)
		{
			return;
		}
		
		size_type line, column;
		line_column_from_buffer_position<traits_type>(source, buffer_position, line, column);
		error_function(type, ch, line, column);
		
		error_limit--;
	}
	
	buffer_type source;
	ErrorFunc error_function;
	size_type recursion_limit = default_recursion_limit<size_type>;
	size_type error_limit = default_maximum_error_limit<size_type>;
	typename buffer_type::size_type buffer_position = {0};
};

template<typename DocumentViewType, typename ErrorFunc>
struct parser_document_view
{
	using document_type = DocumentViewType;
	using data_type = typename document_type::data_type;
	using view_type = data_type;
	using size_type = typename data_type::size_type;
	using char_type = typename document_type::char_type;
	using traits_type = traits<char_type>;
	using buffer_type = std::basic_string<char_type>;
	
	data_type make_data()
	{
		// document_view uses string_view to point directly into the source buffer.
		// Therefore we need to point the data inside the buffer
		return data_type(source.data()+buffer_position, 0);
	}
	
	void add_character(data_type& dst, char_type ch)
	{
		const auto offset = dst.data()-source.data() + dst.size();
		source.data()[offset] = ch;
		dst = data_type(dst.data(), dst.size()+1);
	}
	void copy_string(data_type& dst, view_type src)
	{
		const auto offset = dst.data()-source.data() + dst.size();
		// If they perfectly overlap already we don't have to do anything
		if(dst.data() != src.data())
		{
			std::copy(src.data(), src.data()+src.size(), source.data()+offset);
		}
		dst = data_type(dst.data(), dst.size() + src.size());
	}
	void resize_string(data_type& dst, size_type n_bytes)
	{
		dst = data_type(dst.data(), n_bytes);
	}
	char_type* data_of(data_type& str)
	{
		// We need to get a modifiable pointer. But because each data is stored as
		// a string_view inside the source, we need to get the offset of this data
		return source.data()+(str.data() - source.data());
	}
	
	void next_character()
	{
		++buffer_position;
	}
	char_type current_character() const
	{
		return source[buffer_position];
	}
	char_type peek_next_character() const
	{
		return next_eof() ? traits_type::c_nullchar : source[buffer_position+1];
	}
	bool eof() const { return buffer_position >= source.size(); }
	bool next_eof() const { return buffer_position+1 >= source.size(); }
	
	void error(parser_error type, char_type ch, size_type buffer_position)
	{
		if(error_limit == 0)
		{
			return;
		}
		
		size_type line, column;
		line_column_from_buffer_position<traits_type>(source, buffer_position, line, column);
		error_function(type, ch, line, column);
		
		error_limit--;
	}
	
	buffer_type& source;
	ErrorFunc error_function;
	size_type recursion_limit = default_recursion_limit<size_type>;
	size_type error_limit = default_maximum_error_limit<size_type>;
	typename buffer_type::size_type buffer_position = {0};
};

}

#endif
