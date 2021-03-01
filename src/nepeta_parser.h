// Part of the nepeta project.
// Original source: https://git.sepiamori.com/sepia-mori/nepeta
//
//          Copyright Sepia Mori AS 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
#ifndef NEPETA_NEPETA_PARSER_H
#define NEPETA_NEPETA_PARSER_H

#include <string>

#include "nepeta.h"
#include "nepeta_parser_detail.h"
#include "nepeta_parser_specialization.h"

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

#endif
