// Part of the nepeta project.
// Original source: https://git.sepiamori.com/sepia-mori/nepeta
//
//          Copyright Sepia Mori AS 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
#ifndef NEPETA_NEPETA_WRITER_H
#define NEPETA_NEPETA_WRITER_H

#include <sstream>
#include <limits>

#include "nepeta.h"

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

#include "nepeta_writer_detail.h"

#endif
