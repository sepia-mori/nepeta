// Part of the nepeta project.
// Original source: https://git.sepiamori.com/sepia-mori/nepeta
//
//          Copyright Sepia Mori AS 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
#ifndef NEPETA_NEPETA_H
#define NEPETA_NEPETA_H
namespace nepeta { constexpr bool is_self_contained = false; }

#include <string>
#include <string_view>
#include <vector>
#include <tuple>

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

#include "nepeta_parser.h"
#include "nepeta_writer.h"
#include "nepeta_algorithm.h"

#endif
