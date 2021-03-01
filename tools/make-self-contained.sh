#!/bin/bash

HEADER_PATH="$1"
IFS=''

cat << EOF
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

EOF

for fstr in nepeta nepeta_traits nepeta_base64 nepeta_parser_specialization nepeta_parser_detail nepeta_parser nepeta_writer nepeta_writer_detail nepeta_algorithm
do
	echo "// Beginning of $fstr.h"
	tail --lines=+11 "$HEADER_PATH/$fstr.h" | head --lines=-2 | while read -r line
	do
		# Skip include directives
		[[ $line =~ ^\#include.*$ ]] && continue
		
		echo "$line"
	done
	echo "// End of $fstr.h"
done

echo "#endif"

