// Part of the nepeta project.
// Original source: https://git.sepiamori.com/sepia-mori/nepeta
//
//          Copyright Sepia Mori AS 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
#ifndef NEPETA_NEPETA_TRAITS_H
#define NEPETA_NEPETA_TRAITS_H

#include <string_view>

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

#endif
