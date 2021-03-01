// Part of the nepeta project.
// Original source: https://git.sepiamori.com/sepia-mori/nepeta
//
//          Copyright Sepia Mori AS 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef NEPETA_H_SELF_CONTAINED
#define NEPETA_H_SELF_CONTAINED

#include <algorithm>
#include <cstdint>
#include <limits>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

namespace nepeta { constexpr bool is_self_contained = true; }

// Beginning of nepeta.h


/**
 * \brief Nepeta namespace
 */
namespace nepeta
{

/// Current major version
constexpr int version_major = 0;

/// Current minor version
constexpr int version_minor = 9;

/// Current revision
constexpr int version_revision = 0;

/// \addtogroup parsing
//@{
/**
 * \brief Errors and warnings that can be raised during parsing
 * 
 * Error handlers must support a function call on the form:
 * \code{.cpp}
 * void error_handler(parser_error type, char_type character, size_type line, size_type column);
 * \endcode
 */
enum class parser_error
{
	/// An illegal character was detected during parsing and ignored, this character is passed to the error handler.
	/// Line and column will refer to the location of the character.
	illegal_character				= 0,
	/// A nested node was never closed. Line and column will refer to the beginning of the unclosed node.
	node_not_closed					= 1,
	/// A comment was never closed. Line and column will refer to the beginning of the unclosed comment.
	comment_not_closed				= 2,
	/// A string was never closed. Line and column will refer to the beginning of the unclosed string.
	string_not_closed				= 3,
	/// A block was never closed. Line and column will refer to the opening '{' of the block.
	block_not_closed				= 4,
	/// Too many '#' markers were used to close nodes. Line and column will refer to the location of the mark.
	too_may_node_closing_markers	= 5,
	/// An invalid codec was passed to a block, and "text" will be assumed. Only "text" and "base64" are valid.
	/// Line and column will refer to the start of the codec string.
	bad_codec						= 6,
	/// Raised when too many nested nodes have been reached.
	/// This prevents crashing due to a stack overflow.
	/// This limit can be changed by either directly modifying \c default_recursion_limit, or
	/// Manually creating instances of \c parser_document or \c parser_document_view and
	/// setting the recursion limit for them.
	/// Line and column points to the start of the nested node that exceeded the limit.
	recursion_limit_reached			= 7,
	/// Warns of a line that should end with whitespace, had an unexpected character.
	/// The unexpected character is passed to the error handler.
	/// Line and column will refer to the location of the character.
	require_newline					= 8,
	/// An unexpected escape code was detected and ignored.
	/// The unexpected character is passed to the error handler.
	/// Line and column will refer to the location of the character.
	invalid_escape					= 9,
	/// A '}' character was located at the indentation of block text, rather than one indentation level below.
	/// The character will be included, but a warning is raised in case this was not intended.
	/// This character should instead be escaped to prevent issues.
	/// Line and column refers to the location of the bad '}'.
	bad_block_close					= 10,
	
	/// The first warning in this enum. All warnings are >= warning_start.
	warning_start					= require_newline
};
//@}

/**
 * \defgroup document Document
 * 
 * \brief Document objects
 */
/// \addtogroup document
//@{

/**
 * \brief Basic nepeta document, using std::basic_string for storage.
 * 
 * \note Prefer \link document \endlink for typical use cases.
 * 
 * \sa document
 * \sa load
 * \sa load_into
 * \sa basic_document_view
 */
template<typename CharT>
class basic_document
{
public:
	/// Data type used to store data in a document
	using data_type = std::basic_string<CharT>;
	/// Data type used for returning a view to data in the document
	using view_type = std::basic_string_view<CharT>;
	/// Defines the underlying character type used for data
	using char_type = CharT;
	/// Defines how the sequence of data is stored
	using data_vector = std::vector<data_type>;
	/// Defines how the sequence of children is stored
	using child_vector = std::vector<basic_document>;
	
	auto tie() const
	{
		return std::tie(id, data, children);
	}
	
	/// The ID of this node.
	/// This is normally empty for the root node.
	data_type id = "";
	/// A sequence of \c data_type elements.
	/// This is normally empty for the root node.
	data_vector data = {};
	/// A sequence of nested children of this node.
	/// The nested nodes have the same data type, making the document a nested tree of documents.
	child_vector children = {};
};

/**
 * \brief Basic nepeta document view, using std::basic_string_view for storage.
 * 
 * \note Prefer \link document_view \endlink for typical use cases.
 * 
 * \warning In comparison to a normal document, this object stores data as string views.
 * This means that it is the responsibility of the user of basic_document_view to manage the memory
 * of whatever the string views point to.
 * 
 * \sa document_view
 * \sa load_view
 * \sa load_into_view
 * \sa basic_document
 */
template<typename CharT>
class basic_document_view
{
public:
	/// Data type used to store data in a document
	using data_type = std::basic_string_view<CharT>;
	/// Data type used for returning a view to data in the document
	using view_type = data_type;
	/// Defines the underlying character type used for data
	using char_type = CharT;
	/// Defines how the sequence of data is stored
	using data_vector = std::vector<data_type>;
	/// Defines how the sequence of children is stored
	using child_vector = std::vector<basic_document_view>;
	
	auto tie() const
	{
		return std::tie(id, data, children);
	}
	
	/// The ID of this node.
	/// This is normally empty for the root node.
	data_type id = "";
	/// A sequence of \c data_type elements.
	/// This is normally empty for the root node.
	data_vector data = {};
	/// A sequence of nested children of this node.
	/// The nested nodes have the same data type, making the document a nested tree of documents.
	child_vector children = {};
};

/**
 * \brief A \c basic_document using 'char' for storage.
 * 
 * For all typical usage, this alias should be prefered over using \c basic_document directly
 */
using document = basic_document<char>;
/**
 * \brief A \c basic_document_view using 'char' for storage.
 * 
 * For all typical usage, this alias should be prefered over using \c basic_document_view directly
 */
using document_view = basic_document_view<char>;

template<typename CharT>
bool operator==(const basic_document_view<CharT>& a, const basic_document_view<CharT>& b)
{
	return a.tie() == b.tie();
}
template<typename CharT>
bool operator!=(const basic_document_view<CharT>& a, const basic_document_view<CharT>& b)
{
	return a.tie() != b.tie();
}
template<typename CharT>
bool operator<(const basic_document_view<CharT>& a, const basic_document_view<CharT>& b)
{
	return a.tie() < b.tie();
}
template<typename CharT>
bool operator<=(const basic_document_view<CharT>& a, const basic_document_view<CharT>& b)
{
	return a.tie() <= b.tie();
}
template<typename CharT>
bool operator>=(const basic_document_view<CharT>& a, const basic_document_view<CharT>& b)
{
	return a.tie() >= b.tie();
}
template<typename CharT>
bool operator>(const basic_document_view<CharT>& a, const basic_document_view<CharT>& b)
{
	return a.tie() > b.tie();
}

template<typename CharT>
bool operator==(const basic_document<CharT>& a, const basic_document<CharT>& b)
{
	return a.tie() == b.tie();
}
template<typename CharT>
bool operator!=(const basic_document<CharT>& a, const basic_document<CharT>& b)
{
	return a.tie() != b.tie();
}
template<typename CharT>
bool operator<(const basic_document<CharT>& a, const basic_document<CharT>& b)
{
	return a.tie() < b.tie();
}
template<typename CharT>
bool operator<=(const basic_document<CharT>& a, const basic_document<CharT>& b)
{
	return a.tie() <= b.tie();
}
template<typename CharT>
bool operator>=(const basic_document<CharT>& a, const basic_document<CharT>& b)
{
	return a.tie() >= b.tie();
}
template<typename CharT>
bool operator>(const basic_document<CharT>& a, const basic_document<CharT>& b)
{
	return a.tie() > b.tie();
}

//@}

}

// End of nepeta.h
// Beginning of nepeta_traits.h

namespace nepeta::detail
{

template<typename CharT>
struct traits
{
};

template<typename Traits, typename CharT>
struct base_traits_logic
{
	using char_type = CharT;
	
	constexpr static bool is_whitespace(char_type ch)
	{
		return ch == Traits::c_space || ch == Traits::c_tab;
	}
	
	constexpr static bool is_newline(char_type ch)
	{
		return ch == Traits::c_newline || ch == Traits::c_carriage_return;
	}
	
	constexpr static bool is_binary(char_type ch)
	{
		return ch >= Traits::c_nullchar && ch < Traits::c_space && !is_whitespace(ch) && !is_newline(ch);
	}
	
	constexpr static bool is_end_of_data_context_marker(char_type ch)
	{
		return ch == Traits::c_end_of_data_context;
	}
	
	constexpr static bool is_continue_data_context_on_next_line_marker(char_type ch)
	{
		return ch == Traits::c_backslash;
	}
	
	constexpr static bool is_comment_marker(char_type ch)
	{
		return ch == Traits::c_comment_marker;
	}
	
	constexpr static bool is_comment_multiline_marker(char_type ch)
	{
		return ch == Traits::c_comment_multiline_marker;
	}
	
	constexpr static bool is_string_marker(char_type ch)
	{
		return ch == Traits::c_double_quote;
	}
	
	constexpr static bool is_block_start_marker(char_type ch)
	{
		return ch == Traits::c_block_data_open;
	}
	
	constexpr static bool is_block_end_marker(char_type ch)
	{
		return ch == Traits::c_block_data_close;
	}
	
	constexpr static bool is_nested_node_marker(char_type ch)
	{
		return ch == Traits::c_nested_node_marker;
	}
	
	constexpr static bool is_escape_character_marker(char_type ch)
	{
		return ch == Traits::c_backslash;
	}
	
	// Identifiers are plain text without any of the following:
	// - Control/binary characters
	// - Whitespace or newline
	// - Any of the special characters: #"{}\;
	constexpr static bool is_identifier(char_type ch)
	{
		return
			!is_binary(ch) &&
			!is_whitespace(ch) &&
			!is_newline(ch) &&
			!is_nested_node_marker(ch) &&
			!is_string_marker(ch) &&
			!is_end_of_data_context_marker(ch) &&
			!is_continue_data_context_on_next_line_marker(ch) &&
			!is_block_start_marker(ch) &&
			!is_block_end_marker(ch);
	}
	
	constexpr static bool is_minus_sign(char_type ch)
	{
		return ch == Traits::c_minus_sign;
	}
	
	constexpr static bool is_plus_sign(char_type ch)
	{
		return ch == Traits::c_plus_sign;
	}
	
	constexpr static bool is_number_spacer(char_type ch)
	{
		return ch == Traits::c_single_quote;
	}
	
	constexpr static bool is_base64_padding_character(char_type ch)
	{
		return ch == Traits::base64_padding_character;
	}
};

template<>
struct traits<char> : public base_traits_logic<traits<char>, char>
{
	constexpr static char c_nullchar = '\0';
	constexpr static char c_audible_bell = '\a';
	constexpr static char c_backspace = '\b';
	constexpr static char c_form_feed = '\f';
	constexpr static char c_newline = '\n';
	constexpr static char c_carriage_return = '\r';
	constexpr static char c_horizontal_tab = '\t';
	constexpr static char c_vertical_tab = '\v';
	constexpr static char c_single_quote = '\'';
	constexpr static char c_double_quote = '\"';
	constexpr static char c_backslash = '\\';
	constexpr static char c_space = ' ';
	constexpr static char c_tab = '\t';
	constexpr static char c_block_data_open = '{';
	constexpr static char c_block_data_close = '}';
	constexpr static char c_end_of_data_context = ';';
	constexpr static char c_comment_marker = '/';
	constexpr static char c_comment_multiline_marker = '*';
	constexpr static char c_nested_node_marker = '#';
	constexpr static char c_plus_sign = '+';
	constexpr static char c_minus_sign = '-';
	
	constexpr static char esc_nullchar = '0';
	constexpr static char esc_audible_bell = 'a';
	constexpr static char esc_backspace = 'b';
	constexpr static char esc_form_feed = 'f';
	constexpr static char esc_newline = 'n';
	constexpr static char esc_carriage_return = 'r';
	constexpr static char esc_tab = 't';
	constexpr static char esc_vertical_tab = 'v';
	
	constexpr static std::string_view true_value = "true";
	constexpr static std::string_view false_value = "false";
	
	constexpr static std::string_view base64_encoding_table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	constexpr static char base64_padding_character = '=';
	constexpr static std::string_view base64_codec = "base64";
	constexpr static std::string_view text_codec = "text";
	
	constexpr static bool is_number(char_type ch)
	{
		return ch >= '0' && ch <= '9';
	}
	
	constexpr static unsigned int to_number(char_type ch)
	{
		return ch - '0';
	}
};

}
// End of nepeta_traits.h
// Beginning of nepeta_base64.h


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
// End of nepeta_base64.h
// Beginning of nepeta_parser_specialization.h


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
// End of nepeta_parser_specialization.h
// Beginning of nepeta_parser_detail.h

/// \internal
namespace nepeta::detail
{

template<typename ParserSpecialization>
struct nepeta_parser
{
	enum class parsing_position
	{
		eof,
		valid
	};
	
	enum class block_codec_type
	{
		text,
		base64
	};
	
	enum class comment_type
	{
		passed_to_next_line,
		stayed_on_same_line,
		not_a_comment
	};
	
	enum class parser_data_type
	{
		identifier,
		string,
		block,
		
		not_a_data_type
	};
	
	using document_type = typename ParserSpecialization::document_type;
	using data_type = typename ParserSpecialization::data_type;
	using view_type = typename ParserSpecialization::view_type;
	using size_type = typename ParserSpecialization::size_type;
	using char_type = typename ParserSpecialization::char_type;
	using traits_type = typename ParserSpecialization::traits_type;
	
	constexpr static auto whitespace_and_newlines = [](char_type cur) { return traits_type::is_whitespace(cur) || traits_type::is_newline(cur); };
	constexpr static auto whitespace = [](char_type cur) { return traits_type::is_whitespace(cur); };
	constexpr static auto identifier = [](char_type cur) { return traits_type::is_identifier(cur); };
	constexpr static auto block_text = [](char_type cur) { return !traits_type::is_newline(cur) && !traits_type::is_escape_character_marker(cur); };
	constexpr static auto until_newline = [](char_type cur) { return !traits_type::is_newline(cur); };
	
	nepeta_parser(ParserSpecialization&& specialization_p)
		: specialization(std::move(specialization_p))
		, current_character(is_eof() ? traits_type::c_nullchar : specialization.current_character())
	{
	}
	
	// Main parsing contexts
	void parse_node_body_context(document_type& node, size_type current_recursion = 0, size_type start_pos = 0);
	document_type& parse_node_header_context(document_type& node, parser_data_type header_data_type);
	void parse_node_data_context(document_type& node);
	data_type parse_single_data(parser_data_type type);
	void parse_data_block_codec_context(document_type& node);
	
	// Data parsing contexts
	data_type parse_data_identifier_context();
	data_type parse_data_string_context();
	data_type parse_data_block_context();
	
	// Block parsing contexts
	data_type parse_data_block_body_context(size_type start_position, bool is_base64);
	block_codec_type parse_data_block_codec_context();
	void parse_block_line(data_type& data);
	
	// Helper functions
	comment_type skip_comment();
	void skip_whitespace_until_newline();
	
	// Skipping of multiple characters
	template<typename Func>
	parsing_position skip(Func&& matching_predicate)
	{
		while(matching_predicate(current_character))
		{
			next_character();
			
			if(is_eof())
			{
				return parsing_position::eof;
			}
			
			update_current_character_unsafe();
		}
		
		return is_eof() ? parsing_position::eof : parsing_position::valid;
	}
	
	template<typename Func>
	[[nodiscard]] size_type read(Func&& matching_predicate)
	{
		const auto start = get_buffer_position();
		skip(std::forward<Func>(matching_predicate));
		return get_buffer_position()-start;
	}
	
	// Utility
	void read_escape_character(data_type& data);
	parser_data_type detect_data_type() const
	{
		if(traits_type::is_identifier(current_character))
		{
			return parser_data_type::identifier;
		}
		else if(traits_type::is_string_marker(current_character))
		{
			return parser_data_type::string;
		}
		else if(traits_type::is_block_start_marker(current_character))
		{
			return parser_data_type::block;
		}
		else
		{
			return parser_data_type::not_a_data_type;
		}
	}
	
	/// Specialization interface
	//@{
		char_type peek_next_character() const
		{
			return specialization.peek_next_character();
		}
		
		/**
		 * Moves parsing to the next character, handling windows-style newline
		 * if necessary.
		 */
		void next_character()
		{
			if(current_character == traits_type::c_carriage_return && peek_next_character() == traits_type::c_newline)
			{
				specialization.next_character();
			}
			
			specialization.next_character();
		}
		
		void update_current_character_unsafe()
		{
			current_character = specialization.current_character();
		}
		
		void update_current_character()
		{
			current_character = is_eof() ? traits_type::c_nullchar : specialization.current_character();
		}
		
		bool is_eof() const { return specialization.eof(); }
		size_type get_buffer_position() const { return specialization.buffer_position; }
		size_type get_recursion_limit() const { return specialization.recursion_limit; }
		data_type make_data() { return specialization.make_data(); }
		
		void append_buffer_string(data_type& data, size_type n_bytes)
		{
			specialization.copy_string(data, source_view_before_current(n_bytes));
		}
		
		void append_buffer_character(data_type& data, char_type ch)
		{
			specialization.add_character(data, ch);
		}
		
		view_type source_view_before_current(const size_type bytes)
		{
			return view_type(specialization.source.data()+specialization.buffer_position-bytes, bytes);
		}
		
		void base64_convert(data_type& data)
		{
			const auto new_size = convert_from_base64(specialization.data_of(data), data.size());
			specialization.resize_string(data, new_size);
		}
		
		// Error handling
		void error(parser_error type, char_type ch, size_type buffer_pos)
		{
			specialization.error(type, ch, buffer_pos);
		}
	//@}
	
	ParserSpecialization specialization;
	char_type current_character;
};

template<typename ParserSpecialization>
void nepeta_parser<ParserSpecialization>::parse_node_body_context(document_type& node, size_type current_recursion, size_type start_pos)
{
	// Skip past whitespace and newlines whenever possible
	while( skip(whitespace_and_newlines) == parsing_position::valid )
	{
		// First try to skip comments. If this fails we try any other parsing
		if(traits_type::is_comment_marker(current_character) && skip_comment() != comment_type::not_a_comment)
		{
			// Comment was skipped
		}
		// Otherwise there is a plain node here
		else
		{
			const auto nested_start_pos = get_buffer_position();
			
			const bool is_nested = traits_type::is_nested_node_marker(current_character);
			if(is_nested)
			{
				next_character();
				update_current_character();
				
				if(is_eof() || traits_type::is_whitespace(current_character) || traits_type::is_newline(current_character))
				{
					// End of the current node
					if(current_recursion == size_type{0})
					{
						error(parser_error::too_may_node_closing_markers, traits_type::c_nullchar, nested_start_pos);
					}
					return;
				}
			}
			
			const parser_data_type header_data_type = detect_data_type();
			if(header_data_type == parser_data_type::string || header_data_type == parser_data_type::identifier)
			{
				auto& child = parse_node_header_context(node, header_data_type);
				if(current_recursion + size_type{1} >= get_recursion_limit())
				{
					error(parser_error::recursion_limit_reached, traits_type::c_nullchar, nested_start_pos);
					skip(until_newline);
				}
				else
				{
					if(is_nested)
					{
						parse_node_body_context(child, current_recursion + size_type{1}, nested_start_pos);
					}
				}
			}
			else
			{
				error(parser_error::illegal_character, current_character, get_buffer_position());
				if(!traits_type::is_nested_node_marker(current_character))
				{
					next_character();
					update_current_character();
				}
			}
		}
	}
	
	// At this point EOF is reached
	if(current_recursion > size_type{0})
	{
		error(parser_error::node_not_closed, traits_type::c_nullchar, start_pos);
	}
}

template<typename ParserSpecialization>
auto nepeta_parser<ParserSpecialization>::parse_node_header_context(document_type& node, parser_data_type header_data_type) -> document_type&
{
	document_type& tmp = node.children.emplace_back();
	tmp.id = parse_single_data(header_data_type);
	parse_node_data_context(tmp);
	return tmp;
}

template<typename ParserSpecialization>
void nepeta_parser<ParserSpecialization>::parse_node_data_context(document_type& node)
{
	while( skip(whitespace) == parsing_position::valid )
	{
		comment_type type;
		if( traits_type::is_comment_marker(current_character) && (type=skip_comment()) != comment_type::not_a_comment )
		{
			// Comments passing to the next line aborts the data context
			if(type == comment_type::passed_to_next_line)
			{
				return;
			}
		}
		else if( traits_type::is_end_of_data_context_marker(current_character) )
		{
			// Semicolon always aborts a data context
			next_character();
			update_current_character();
			return;
		}
		else if( traits_type::is_continue_data_context_on_next_line_marker(current_character) )
		{
			next_character();
			update_current_character();
			skip_whitespace_until_newline();
		}
		else if( traits_type::is_newline(current_character) )
		{
			return;
		}
		else
		{
			const auto type = detect_data_type();
			if(type == parser_data_type::not_a_data_type)
			{
				error(parser_error::illegal_character, current_character, get_buffer_position());
				next_character();
				update_current_character();
			}
			else
			{
				node.data.emplace_back(parse_single_data(type));
			}
		}
	}
}

template<typename ParserSpecialization>
auto nepeta_parser<ParserSpecialization>::parse_single_data(parser_data_type type) -> data_type
{
	switch(type)
	{
		case parser_data_type::block:
			return parse_data_block_context();
		case parser_data_type::string:
			return parse_data_string_context();
		case parser_data_type::identifier:
			return parse_data_identifier_context();
		default:
			// This should never happen
			error(parser_error::illegal_character, current_character, get_buffer_position());
			next_character();
			update_current_character();
			return make_data();
	}
}

// Data parsing contexts
template<typename ParserSpecialization>
auto nepeta_parser<ParserSpecialization>::parse_data_identifier_context() -> data_type
{
	auto data = make_data();
	append_buffer_string(data, read(identifier));
	return data;
}

template<typename ParserSpecialization>
auto nepeta_parser<ParserSpecialization>::parse_data_string_context() -> data_type
{
	const auto start_position = get_buffer_position();
	
	// Skip initial string marker
	next_character();
	
	auto data = make_data();
	while (!is_eof())
	{
		update_current_character_unsafe();
		
		// Skip as many bytes as possible. Stop when string-marker, newline, or escape character is reached.
		const auto bytes = read([](char_type cur)
		{
			return !(traits_type::is_string_marker(cur) || traits_type::is_newline(cur) || traits_type::is_escape_character_marker(cur));
		});
		append_buffer_string(data, bytes);
		
		// Check why skipping stopped
		if( traits_type::is_string_marker(current_character) )
		{
			// End of string
			next_character();
			update_current_character();
			return data;
		}
		else if( traits_type::is_escape_character_marker(current_character) )
		{
			read_escape_character(data);
		}
		else
		{
			// Newline
			break;
		}
	}
	
	error(parser_error::string_not_closed, traits_type::c_nullchar, start_position);
	return data;
}

template<typename ParserSpecialization>
auto nepeta_parser<ParserSpecialization>::parse_data_block_context() -> data_type
{
	const auto start_position = get_buffer_position();
	
	// Skip over the known '{'
	next_character();
	update_current_character();
	
	// Skip whitespace either until newline, or a codec is present
	if(skip(whitespace) == parsing_position::eof)
	{
		error(parser_error::block_not_closed, traits_type::c_nullchar, start_position);
		return make_data();
	}
	
	// Parse the codec if present
	const bool process_as_base64 = traits_type::is_identifier(current_character) && parse_data_block_codec_context() == block_codec_type::base64;
	
	// Skip to the beginning of the next line
	skip_whitespace_until_newline();
	
	// Parse the block itself
	auto data = parse_data_block_body_context(start_position, process_as_base64);
	if(process_as_base64)
	{
		// TODO: Errors if conversion has issues. Or more resilient base64 decoding
		base64_convert(data);
	}
	
	return data;
}

template<typename ParserSpecialization>
auto nepeta_parser<ParserSpecialization>::parse_data_block_codec_context() -> block_codec_type
{
	const auto codec_start_position = get_buffer_position();
	const auto codec = source_view_before_current(read(identifier));
	
	if( codec == traits_type::base64_codec )
	{
		return block_codec_type::base64;
	}
	if( codec != traits_type::text_codec )
	{
		error(parser_error::bad_codec, traits_type::c_nullchar, codec_start_position);
		// Try to recover from error here.. Even if there is a bad codec name we can still continue
	}
	
	return block_codec_type::text;
}



// Special parsing contexts
template<typename ParserSpecialization>
auto nepeta_parser<ParserSpecialization>::parse_data_block_body_context(size_type start_position, bool is_base64) -> data_type
{
	// First line we must figure out the indentation.
	// The indentation lasts until the first non-space character, and is consistent throughout the rest of the block.
	const auto before_whitespace = get_buffer_position();
	skip(whitespace);
	const auto column_depth = get_buffer_position()-before_whitespace;
	
	data_type data = make_data();
	
	bool is_first_line = true;
	auto current_column_start = before_whitespace;
	while( !is_eof() )
	{
		// Abort if end character at start of line
		// Note that bad indentation (non-whitespace) is possible but strictly
		// ill-formed, except for the end marker, which should always be at least one
		// indentation level lower than the text
		if( traits_type::is_block_end_marker(current_character) )
		{
			// Closing the block on the first line is always valid
			if( (get_buffer_position()-current_column_start) < column_depth || is_first_line )
			{
				next_character();
				update_current_character();
				return data;
			}
			else
			{
				error(parser_error::bad_block_close, traits_type::c_nullchar, get_buffer_position());
			}
		}
		
		if(!is_first_line && !is_base64)
		{
			append_buffer_character(data, ParserSpecialization::traits_type::c_newline);
		}
		
		// Parse a single line for a block, until the newline. This takes care of escape characters as well
		parse_block_line(data);
		
		// Skip indentation on the next line
		// This either breaks when the desired column is reached, or a non-whitespace character is reached
		current_column_start = get_buffer_position();
		const auto target_buffer_position = current_column_start+column_depth;
		skip([this, &target_buffer_position](char_type cur) {
			return get_buffer_position() < target_buffer_position && traits_type::is_whitespace(cur);
		});
		
		is_first_line = false;
	}
	
	error(parser_error::block_not_closed, traits_type::c_nullchar, start_position);
	return data;
}

template<typename ParserSpecialization>
void nepeta_parser<ParserSpecialization>::parse_block_line(data_type& data)
{
	while(!is_eof())
	{
		// Read as much block data as possible. Stop on either newline or escape character
		append_buffer_string(data, read(block_text));
		
		if(is_eof())
		{
			break;
		}
		else if(traits_type::is_newline(current_character))
		{
			next_character();
			update_current_character();
			return;
		}
		else if(traits_type::is_escape_character_marker(current_character))
		{
			read_escape_character(data);
		}
	}
}

// Skipping of multiple characters
template<typename ParserSpecialization>
void nepeta_parser<ParserSpecialization>::skip_whitespace_until_newline()
{
	skip(whitespace);
	
	if(!traits_type::is_newline(current_character))
	{
		// We found an unexpected character here
		// Notify the error and continue
		error(parser_error::require_newline, current_character, get_buffer_position());
		skip(until_newline);
	}
	
	if(!is_eof())
	{
		next_character();
		update_current_character();
	}
}

template<typename ParserSpecialization>
auto nepeta_parser<ParserSpecialization>::skip_comment() -> comment_type
{
	const auto start_position = get_buffer_position();
	
	const auto next_ch = peek_next_character();
	if(traits_type::is_comment_multiline_marker(next_ch))
	{
		// Skip over the opening '/*'
		next_character();
		next_character();
		
		comment_type type = comment_type::stayed_on_same_line;
		while(!is_eof())
		{
			update_current_character_unsafe();
			
			// Checking for closing comment '*/'
			if(traits_type::is_comment_multiline_marker(current_character) && traits_type::is_comment_marker(peek_next_character()))
			{
				next_character();
				next_character();
				update_current_character();
				return type;
			}
			else if(traits_type::is_newline(current_character))
			{
				type = comment_type::passed_to_next_line;
			}
			
			next_character();
		}
		
		error(parser_error::comment_not_closed, traits_type::c_nullchar, start_position);
		return type;
	}
	else if(traits_type::is_comment_marker(next_ch))
	{
		skip(until_newline);
		return comment_type::passed_to_next_line;
	}
	else
	{
		return comment_type::not_a_comment;
	}
}

// Utility functions
template<typename ParserSpecialization>
void nepeta_parser<ParserSpecialization>::read_escape_character(data_type& data)
{
	// Skip over the known escape character marker
	next_character();
	update_current_character();
	
	char_type to_add = traits_type::c_nullchar;
	using tr = traits_type;
	switch(current_character)
	{
		
		case tr::esc_nullchar:			to_add = tr::c_nullchar;		break;
		case tr::esc_audible_bell:		to_add = tr::c_audible_bell;	break;
		case tr::esc_backspace:			to_add = tr::c_backspace;		break;
		case tr::esc_form_feed:			to_add = tr::c_form_feed;		break;
		case tr::esc_newline:			to_add = tr::c_newline;			break;
		case tr::esc_carriage_return:	to_add = tr::c_carriage_return;	break;
		case tr::esc_tab:				to_add = tr::c_tab;				break;
		case tr::esc_vertical_tab:		to_add = tr::c_vertical_tab;	break;
		// Characters that can be escaped with the character itself
		case tr::c_single_quote:
		case tr::c_double_quote:
		case tr::c_backslash:
		case tr::c_space:
		case tr::c_tab:
		case tr::c_block_data_open:
		case tr::c_block_data_close:
			to_add = current_character;
			break;
		default:
			error(parser_error::invalid_escape, current_character, get_buffer_position());
			return;
	}
	
	append_buffer_character(data, to_add);
	next_character();
	update_current_character();
}

}
// End of nepeta_parser_detail.h
// Beginning of nepeta_parser.h


namespace nepeta
{
namespace detail
{
	constexpr auto empty_error_handler = [](parser_error, auto, auto, auto) {};
}

/**
 * \defgroup parsing Parsing
 * 
 * \brief Functionality related to document parsing
 */

/// \addtogroup parsing
//@{

/**
 * \brief Loads a nepeta document from \p source and appends it to the back of \p doc.
 * 
 * \param doc The nepeta document to append data to
 * \param source A compatible string_view containing nepeta source text
 * \param error_handler Callback to notify any parsing errors. Parameters: void error_handler(parser_error type, char_type character, size_type line, size_type column)
 * 
 * \sa parser_error
 */
template<typename DocumentType, typename ErrorFunc>
inline void load_into(DocumentType& doc, typename DocumentType::view_type source, ErrorFunc&& error_handler)
{
	detail::nepeta_parser parser(detail::parser_document<DocumentType, ErrorFunc>{ source, error_handler });
	parser.parse_node_body_context(doc);
}

/**
 * \brief Loads a nepeta document from \p source and appends it to the back of \p doc, using an empty error handler
 * 
 * \sa load_into
 */
template<typename DocumentType>
inline void load_into(DocumentType& doc, typename DocumentType::view_type source)
{
	return load_into(doc, std::move(source), detail::empty_error_handler);
}

/**
 * \brief Parses \p source and returns a new document from it
 * 
 * \param source A compatible string_view or array containing nepeta source text
 * \param error_handler Callback to notify any parsing errors. Parameters: void error_handler(parser_error type, char_type character, size_type line, size_type column)
 * 
 * \return A \c basic_document compatible with the character type used in SourceType.
 */
template<typename SourceType, typename ErrorFunc>
inline auto load(SourceType&& source, ErrorFunc&& error_handler)
{
	if constexpr(std::is_array_v<std::remove_reference_t<SourceType>>)
	{
		using CharT = std::remove_cv_t<std::remove_pointer_t<std::decay_t<SourceType>>>;
		
		basic_document<CharT> tmp;
		load_into(tmp, std::basic_string_view<CharT>(source, std::size(source)), std::forward<ErrorFunc>(error_handler));
		return tmp;
	}
	else
	{
		using BaseType = std::decay_t<SourceType>;
		
		basic_document<typename BaseType::value_type> tmp;
		load_into(tmp, std::forward<SourceType>(source), std::forward<ErrorFunc>(error_handler));
		return tmp;
	}
}

/**
 * \brief Parses \p source and returns a new document from it, using an empty error handler
 * 
 * \sa load
 * 
 * \return A \c basic_document compatible with the character type used in SourceType.
 */
template<typename SourceType>
inline auto load(SourceType&& source)
{
	return load(std::forward<SourceType>(source), detail::empty_error_handler);
}


/**
 * \brief Intrusively parses from the string \p source and appends it to the back of the document_view \p doc
 * 
 * The parameter \p source is destroyed in the process when parsing, and the document refers directly
 * into the memory of it.
 * 
 * \param doc The nepeta document to append data to
 * \param source A compatible string containing nepeta source text
 * \param error_handler Callback to notify any parsing errors. Parameters: void error_handler(parser_error type, char_type character, size_type line, size_type column)
 * 
 * \warning The document refers directly to the memory in \p source, and the caller is responsible for keeping it in memory, and not changing it
 */
template<typename DocumentViewType, typename ErrorFunc>
inline void load_into_view(DocumentViewType& doc, std::basic_string<typename DocumentViewType::char_type>& source, ErrorFunc&& error_handler)
{
	detail::nepeta_parser parser(detail::parser_document_view<DocumentViewType, ErrorFunc>{ source, error_handler });
	parser.parse_node_body_context(doc);
}

/**
 * \brief Intrusively parses from the string \p source and appends it to the back of the document \p doc, using an empty error handler
 * 
 * \sa load_into_view
 * 
 * \warning The document refers directly to the memory in \p source, and the caller is responsible for keeping it in memory, and not changing it
 */
template<typename DocumentViewType>
inline void load_into_view(DocumentViewType& doc, std::basic_string<typename DocumentViewType::char_type>& source)
{
	return load_into_view(doc, source, detail::empty_error_handler);
}

/**
 * \brief Intrusively parses from the string \p source and returns a new document_view from it
 * 
 * \param source A compatible string containing nepeta source text
 * \param error_handler Callback to notify any parsing errors. Parameters: void error_handler(parser_error type, char_type character, size_type line, size_type column)
 * 
 * \warning The document refers directly to the memory in \p source, and the caller is responsible for keeping it in memory, and not changing it
 */
template<typename CharT, typename ErrorFunc>
inline basic_document_view<CharT> load_view(std::basic_string<CharT>& source, ErrorFunc&& error_handler)
{
	basic_document_view<CharT> tmp;
	load_into_view(tmp, source, error_handler);
	return tmp;
}

/**
 * \brief Intrusively parses from the string \p source and returns a new document_view from it, using an empty error handler
 * 
 * \sa load_view
 * 
 * \warning The document refers directly to the memory in \p source, and the caller is responsible for keeping it in memory, and not changing it
 */
template<typename CharT>
inline basic_document_view<CharT> load_view(std::basic_string<CharT>& source)
{
	return load_view<CharT>(source, detail::empty_error_handler);
}

}

//@}
// End of nepeta_parser.h
// Beginning of nepeta_writer.h


namespace nepeta
{

/**
 * \defgroup writer Writer
 * 
 * \brief Functionality related to writing documents
 */

/// \addtogroup writer
//@{

/**
 * \brief Configurable parameters to use when writing files.
 */
struct writer_parameters
{
	/// Valid types of indentation
	enum class indentation_type
	{
		/// Indentation using tabs
		tabs,
		/// Indentation using spaces
		spaces
	};
	
	/// Which type of indentation to use
	indentation_type indentation = indentation_type::tabs;
	
	/// Number of indentation characters to use when indenting
	unsigned int indentation_characters = 1;
	
	/// Any data larger than this will always be written as a block
	unsigned int limit_for_block_enforcement = 128;
	
	/// \brief Number of characters to check for binary data (requiring base64 encoding).
	/// If no binary data is detected, the writer will write the text with escape
	/// codes instead.
	/// The writer will always check at least up to \c limit_for_block_enforcement as well,
	/// in case we need to write the data as a string
	unsigned int limit_for_checking_binary = std::numeric_limits<unsigned int>::max();
	
	/// Number of base64 characters to write per line, will be rounded up to the nearest multiple of 4
	unsigned int base64_per_line = 60;
};

namespace detail
{
template<typename NepetaType, typename WriterType>
struct nepeta_writer
{
	using data_type = typename NepetaType::data_type;
	using view_type = typename NepetaType::view_type;
	using char_type = typename NepetaType::char_type;
	using size_type = typename data_type::size_type;
	using traits_type = typename detail::traits<char_type>;
	
	enum class target_data_type
	{
		identifier,
		string,
		block,
		block_base64
	};
	
	nepeta_writer(const NepetaType& source, const WriterType& writer_p, const writer_parameters& writer_param_p)
		: writer(writer_p)
		, writer_param(writer_param_p)
		, indentation_char(writer_param.indentation == writer_parameters::indentation_type::tabs ? traits_type::c_tab : traits_type::c_space)
	{
		write_node(source);
	}
	
	// Node and data writing
	void write_node(const NepetaType& node, unsigned int current_depth = 0) const;
	void write_data(const data_type& data, unsigned int current_depth) const;
	void write_string(const data_type& data) const;
	void write_block(const data_type& data, unsigned int current_depth) const;
	void write_base64(const data_type& data, unsigned int current_depth) const;
	
	// Helper functions
	target_data_type determine_identifier_type(const data_type& data) const;
	target_data_type determine_data_type(const data_type& data) const;
	char_type to_escaped_character(char_type ch) const;
	
	// Basic writing functions
	void write_indentation(unsigned int depth) const
	{
		write(indentation_char, writer_param.indentation_characters*depth);
	}
	void write_newline() const
	{
		write(traits_type::c_newline);
	}
	
	void write(char_type ch, unsigned int copies = 1) const
	{
		write(view_type(&ch, 1), copies);
	}
	void write(view_type text, unsigned int copies = 1) const
	{
		writer(text, copies);
	}
	
	const WriterType& writer;
	const writer_parameters& writer_param;
	char_type indentation_char;
};
}

/**
 * \brief Writes \c source to a stream
 * 
 * \param stream Compatible stream to write the source to
 * \param source The source nepeta document to write
 * \param param Optional writer parameters to use when writing the string
 */
template<typename NepetaType, typename Stream>
void write_to_stream(Stream& stream, const NepetaType& source, const writer_parameters& param = {})
{
	const auto stream_writer = [&stream](typename NepetaType::view_type text, unsigned int copies)
	{
		while(copies > 0)
		{
			stream.write(text.data(), text.size());
			copies--;
		}
	};
	
	detail::nepeta_writer(source, stream_writer, param);
}

/**
 * \brief Writes \c source in-place to a string
 * 
 * \sa write_to_string(const NepetaType&, const writer_parameters&)
 * 
 * \param str String to write the document to
 * \param source The source nepeta document to write
 * \param param Optional writer parameters to use when writing the string
 */
template<typename NepetaType>
void write_to_string(std::basic_string<typename NepetaType::char_type>& str, const NepetaType& source, const writer_parameters& param = {})
{
	const auto string_writer = [&str](typename NepetaType::view_type text, unsigned int copies)
	{
		while(copies > 0)
		{
			str.append(text);
			copies--;
		}
	};
	
	detail::nepeta_writer(source, string_writer, param);
}

/**
 * \brief Writes \c source to a string, and returns this string
 * 
 * \sa write_to_string(std::basic_string<typename NepetaType::char_type>&, const NepetaType&, const writer_parameters&)
 * 
 * \param source The source nepeta document to write
 * \param param Optional writer parameters to use when writing the string
 * 
 * \return A \c std::basic_string compatible with the passed NepetaType
 */
template<typename NepetaType>
auto write_to_string(const NepetaType& source, const writer_parameters& param = {})
{
	std::basic_string<typename NepetaType::char_type> str;
	write_to_string(str, source, param);
	return str;
}

//@}

}

// End of nepeta_writer.h
// Beginning of nepeta_writer_detail.h


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
// End of nepeta_writer_detail.h
// Beginning of nepeta_algorithm.h


namespace nepeta
{

/**
 * \defgroup algorithm Algorithm
 * 
 * \brief Algorithms that operate on documents
 */

/// \addtogroup algorithm
//@{


/**
 * \brief Returns an iterator to the first child in the document which matches the given key.
 * 
 * The iterator returned can not be used to iterate the keys directly.
 * Use the other iteration functions to properly traverse keys.
 * 
 * \sa end
 * \sa next
 */
template<typename DocumentType, typename KeyType>
auto begin(DocumentType& doc, const KeyType& key)
{
	return std::find_if(doc.children.begin(), doc.children.end(), [&](const DocumentType& child)
	{
		return child.id == key;
	});
}

/**
 * \brief Returns an iterator to the last child in the document which matches the given key.
 * 
 * \sa rend
 * \sa rnext
 */
template<typename DocumentType, typename KeyType>
auto rbegin(DocumentType& doc, const KeyType& key)
{
	return std::find_if(doc.children.rbegin(), doc.children.rend(), [&](const DocumentType& child)
	{
		return child.id == key;
	});
}

/**
 * \brief Returns an iterator to the end of the document's children.
 * 
 * \sa begin
 * \sa next
 */
template<typename DocumentType>
auto end(DocumentType& doc)
{
	return doc.children.end();
}

/**
 * \brief Returns a reverse iterator to the end of the document's children.
 * 
 * \sa rbegin
 * \sa rnext
 */
template<typename DocumentType>
auto rend(DocumentType& doc)
{
	return doc.children.rend();
}

/**
 * \brief Returns an iterator to the next matching key.
 * 
 * \sa begin
 * \sa end
 */
template<typename DocumentType, typename IteratorType, typename KeyType>
auto next(DocumentType& doc, const IteratorType& it, const KeyType& key)
{
	return std::find_if(it+1, end(doc), [&](const DocumentType& child)
	{
		return child.id == key;
	});
}

/**
 * \brief Returns an iterator to the next matching key, using reverse iterators.
 * 
 * \sa rbegin
 * \sa rend
 */
template<typename DocumentType, typename IteratorType, typename KeyType>
auto rnext(DocumentType& doc, const IteratorType& it, const KeyType& key)
{
	return std::find_if(it+1, rend(doc), [&](const DocumentType& child)
	{
		return child.id == key;
	});
}

/**
 * \brief Returns a pointer to the first matching child in the document, or nullptr if not found.
 */
template<typename DocumentType, typename KeyType>
DocumentType* find(DocumentType& doc, const KeyType& key)
{
	const auto i = begin(doc, key);
	return i == end(doc) ? nullptr : &(*i);
}

/**
 * \brief Returns a pointer to the last matching child in the document, or nullptr if not found.
 */
template<typename DocumentType, typename KeyType>
DocumentType* rfind(DocumentType& doc, const KeyType& key)
{
	const auto i = rbegin(doc, key);
	return i == rend(doc) ? nullptr : &(*i);
}

/**
 * \brief Iterates through all children matching the given key.
 */
template<typename DocumentType, typename KeyType, typename Func>
void for_each(DocumentType& doc, const KeyType& key, Func&& func)
{
	for(auto i = nepeta::begin(doc, key); i != nepeta::end(doc); i = nepeta::next(doc, i, key))
	{
		func(*i);
	}
}

/**
 * \brief Iterates through all children matching the given key, in reverse order.
 */
template<typename DocumentType, typename KeyType, typename Func>
void rfor_each(DocumentType& doc, const KeyType& key, Func&& func)
{
	for(auto i = nepeta::rbegin(doc, key); i != nepeta::rend(doc); i = nepeta::rnext(doc, i, key))
	{
		func(*i);
	}
}

/**
 * \brief Merges all children and data from \p other into \p doc.
 * 
 * After this function, the data in \p other are appended into
 * the data of \p doc, and the children of \p other are appended onto the children of \p doc.
 */
template<typename DocumentType>
void merge(DocumentType& doc, const DocumentType& other)
{
	std::copy(other.data.begin(), other.data.end(), std::back_inserter(doc.data));
	std::copy(other.children.begin(), other.children.end(), std::back_inserter(doc.children));
}

/**
 * \brief Moves all children and data from \p other and appends them into \p doc.
 * 
 * After this function, the data in \p other are moved and appended into
 * the data of \p doc, and the children of \p other are moved and appended onto the children of \p doc.
 * 
 * \post other.children.empty() == true && other.data.empty() == true
 */
template<typename DocumentType>
void merge(DocumentType& doc, DocumentType&& other)
{
	std::move(other.data.begin(), other.data.end(), std::back_inserter(doc.data));
	std::move(other.children.begin(), other.children.end(), std::back_inserter(doc.children));
	other.data.clear();
	other.children.clear();
}

/**
 * \brief Returns a data element from doc as a view, or std::nullopt if out of bounds.
 */
template<typename DocumentType>
std::optional<typename DocumentType::view_type> opt_data(DocumentType& doc, typename DocumentType::data_vector::size_type index)
{
	return index < doc.data.size() ? std::optional<typename DocumentType::view_type>(doc.data[index]) : std::nullopt;
}

/**
 * \brief Returns true if \c view is equal to the string "true", false if it is equal to the string "false", otherwise std::nullopt is returned.
 */
template<typename ViewType>
std::optional<bool> opt_bool(ViewType&& view)
{
	using traits = detail::traits<typename ViewType::value_type>;
	
	if(view == traits::true_value)
	{
		return true;
	}
	if(view == traits::false_value)
	{
		return false;
	}
	
	return std::nullopt;
}

/**
 * \brief Same as \c opt_bool, but returns a default value instead of std::nullopt.
 * 
 * \sa opt_bool
 */
template<typename ViewType>
bool as_bool(ViewType&& view, bool default_value = false)
{
	return opt_bool(std::forward<ViewType>(view)).value_or(default_value);
}

/**
 * \brief Same as \c opt_bool, but directly indexes the document's data.
 * 
 * \sa opt_bool
 */
template<typename DocumentType>
std::optional<bool> doc_opt_bool(DocumentType& doc, typename DocumentType::child_vector::size_type index)
{
	return index < doc.data.size() ? opt_bool(typename DocumentType::view_type(doc.data[index])) : std::nullopt;
}

/**
 * \brief Same as \c as_bool, but directly indexes the document's data.
 * 
 * \sa as_bool
 */
template<typename DocumentType>
bool doc_as_bool(DocumentType& doc, typename DocumentType::child_vector::size_type index, bool default_value = false)
{
	return doc_opt_bool(doc, index).value_or(default_value);
}

/**
 * \brief Returns \c view converted from base-10 to an integer, or std::nullopt if the integer is invalid.
 * 
 * A valid integer must be compatible with the regex:
 * (-|+)?[0-9']*
 * 
 * Dashes (') are ignored and can be used as separators. If the integer is invalid, std::nullopt is returned.
 * 
 * \note This does not do any bounds checking on the converted integers, and as such the return value of out-of-bounds is undefined.
 * 
 * \return A valid integer, or std::nullopt if malformed. If \c view is empty, 0 is returned.
 */
template<typename IntegerType, typename ViewType>
std::optional<IntegerType> opt_integer(ViewType&& view)
{
	using traits = detail::traits<typename ViewType::value_type>;
	
	auto result = IntegerType{0};
	bool is_negative = false;
	for(decltype(view.size()) i = 0; i < view.size(); ++i)
	{
		const auto ch = view[i];
		if(i == 0 && traits::is_minus_sign(ch))
		{
			is_negative = true;
		}
		else if(i == 0 && traits::is_plus_sign(ch))
		{
			is_negative = false;
		}
		else if(traits::is_number(ch))
		{
			result = result*IntegerType{10} + static_cast<IntegerType>(traits::to_number(ch));
		}
		else if(traits::is_number_spacer(ch))
		{
		}
		else
		{
			// Invalid integer format
			return std::nullopt;
		}
	}
	
	return is_negative ? -result : result;
}

/**
 * \brief Same as \c opt_integer, but returns a default value instead of std::nullopt.
 * 
 * \sa opt_integer
 */
template<typename IntegerType, typename ViewType>
IntegerType as_integer(ViewType&& view, IntegerType default_value = {})
{
	return opt_integer(std::forward<ViewType>(view)).value_or(default_value);
}

/**
 * \brief Same as \c opt_integer, but directly indexes the document's data.
 * 
 * \sa opt_integer
 */
template<typename IntegerType, typename DocumentType>
std::optional<IntegerType> doc_opt_integer(DocumentType& doc,  typename DocumentType::child_vector::size_type index)
{
	return index < doc.data.size() ? opt_integer<IntegerType>(typename DocumentType::view_type(doc.data[index])) : std::nullopt;
}

/**
 * \brief Same as \c as_integer, but directly indexes the document's data.
 * 
 * \sa as_integer
 */
template<typename IntegerType, typename DocumentType>
IntegerType doc_as_integer(DocumentType& doc, typename DocumentType::child_vector::size_type index, IntegerType default_value = {})
{
	return doc_opt_integer<IntegerType>(doc, index).value_or(default_value);
}

}
// End of nepeta_algorithm.h
#endif
