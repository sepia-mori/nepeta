// Part of the nepeta project.
// Original source: https://git.sepiamori.com/sepia-mori/nepeta
//
//          Copyright Sepia Mori AS 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
#ifndef NEPETA_TEST_HELPERS_H
#define NEPETA_TEST_HELPERS_H

#include <functional>

#include <boost/mpl/list.hpp>

#include "nepeta.h"

namespace
{
	using parser_error = nepeta::parser_error;
	using char_type = nepeta::document::char_type;
	
	struct error_entry
	{
		parser_error error;
		char_type character = '\0';
		int line = 0;
		int column = 0;
		
		bool operator==(const error_entry& b) const
		{
			return std::tie(error, character, line, column) == std::tie(b.error, b.character, b.line, b.column);
		}
		
		bool operator!=(const error_entry& b) const
		{
			return !(*this == b);
		}
	};
	
	template<typename T>
	T load_for(std::string& source, const std::function<void(parser_error, char_type, int line, int column)>& cb);
	
	template<>
	nepeta::document_view load_for<nepeta::document_view>(std::string& source, const std::function<void(parser_error, char_type, int line, int column)>& cb)
	{
		return nepeta::load_view(source, cb);
	}
	
	template<>
	nepeta::document load_for<nepeta::document>(std::string& source, const std::function<void(parser_error, char_type, int line, int column)>& cb)
	{
		return nepeta::load(source, cb);
	}
	
	template<typename T>
	struct test_data
	{
		std::string source;
		
		T view;
		T reference;
		std::vector<error_entry> error_v;
		
		static test_data<T> make(std::string source, typename T::child_vector reference_children)
		{
			test_data<T> tmp;
			
			tmp.source = std::move(source);
			tmp.view = load_for<T>(tmp.source,
				[&](parser_error err, char_type ch, int line, int column)
				{
					tmp.error_v.push_back(error_entry{err, ch, line, column});
				}
			);
			tmp.reference = T
			{
				"",
				{},
				std::move(reference_children)
			};
			
			return tmp;
		}
	};
	
	using nepeta_types = boost::mpl::list<nepeta::document, nepeta::document_view>;
}

#endif
