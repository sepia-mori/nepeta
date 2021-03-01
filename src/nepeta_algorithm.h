// Part of the nepeta project.
// Original source: https://git.sepiamori.com/sepia-mori/nepeta
//
//          Copyright Sepia Mori AS 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
#ifndef NEPETA_NEPETA_ALGORITHM_H
#define NEPETA_NEPETA_ALGORITHM_H

#include <algorithm>
#include <optional>

#include "nepeta_traits.h"

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

#endif
