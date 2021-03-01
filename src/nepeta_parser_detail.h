// Part of the nepeta project.
// Original source: https://git.sepiamori.com/sepia-mori/nepeta
//
//          Copyright Sepia Mori AS 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
#ifndef NEPETA_NEPETA_PARSER_DETAIL_H
#define NEPETA_NEPETA_PARSER_DETAIL_H

#include "nepeta.h"
#include "nepeta_base64.h"

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

#endif
