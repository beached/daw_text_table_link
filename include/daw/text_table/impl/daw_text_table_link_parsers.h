// The MIT License (MIT)
//
// Copyright (c) Darrell Wright
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files( the "Software" ), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and / or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include "daw_text_table_assert.h"
#include "daw_text_table_link_common.h"

#include <daw/cpp_17.h>
#include <daw/daw_parser_helper_sv.h>
#include <daw/daw_utility.h>

#include <absl/strings/charconv.h>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cwchar>
#include <type_traits>
#include <utility>

namespace daw::text_data::text_table_details {
	namespace TextTableParserTypes {
		struct String {
			using i_am_a_text_table_parser_type = void;

			template<typename TextTableColumn, typename TableType, typename CharT>
			static constexpr typename TextTableColumn::parse_to
			parse_value( daw::basic_string_view<CharT> rng ) {
				return
				  typename TextTableColumn::constructor{}( rng.data( ), rng.size( ) );
			}
		};

		struct StringRaw {
			using i_am_a_text_table_parser_type = void;

			template<typename TextTableColumn, typename TableType, typename CharT>
			static constexpr typename TextTableColumn::parse_to
			parse_value( daw::basic_string_view<CharT> rng ) {
				return
				  typename TextTableColumn::constructor{}( rng.data( ), rng.size( ) );
			}
		};

		struct Real {
			using i_am_a_text_table_parser_type = void;

			template<typename TextTableColumn, typename TableType, typename CharT>
			static typename TextTableColumn::parse_to
			parse_value( daw::basic_string_view<CharT> rng ) {
				typename TextTableColumn::parse_to result = 0.0;
				auto const abresult = absl::from_chars(
				  rng.data( ), rng.data( ) + static_cast<std::ptrdiff_t>( rng.size( ) ),
				  result );

				switch( abresult.ec ) {
				case std::errc::invalid_argument:
					daw_text_table_error( "Invalid floating point number" );
					break;
				case std::errc::result_out_of_range:
					daw_text_table_error( "Number out of range" );
					break;
				default:
					break;
				}
				return typename TextTableColumn::constructor{}( result );
			}
		};

		struct Unsigned {
			using i_am_a_text_table_parser_type = void;

			template<typename TextTableColumn, typename TableType, typename CharT>
			static constexpr typename TextTableColumn::parse_to
			parse_value( daw::basic_string_view<CharT> rng ) {
				std::uintmax_t result = 0;
				auto dig = static_cast<unsigned>( rng.pop_front( ) ) -
				           static_cast<unsigned>( TableType::zero_char );
				while( dig < 10U ) {
					result *= 10U;
					result += dig;
					dig = static_cast<unsigned>( rng.pop_front( ) ) -
					      static_cast<unsigned>( TableType::zero_char );
				}
				if constexpr( TextTableColumn::range_check ==
				              NumericRangeCheck::CheckForNarrowing ) {
					return typename TextTableColumn::constructor{}(
					  daw::narrow_cast<typename TextTableColumn::parse_to>( result ) );
				} else {
					return typename TextTableColumn::constructor{}(
					  static_cast<typename TextTableColumn::parse_to>( result ) );
				}
			}
		};

		struct Signed {
			using i_am_a_text_table_parser_type = void;
			template<typename TextTableColumn, typename TableType, typename CharT>
			static constexpr typename TextTableColumn::parse_to
			parse_value( daw::basic_string_view<CharT> rng ) {
				std::intmax_t result = 0;
				auto dig = static_cast<unsigned>( rng.pop_front( ) ) -
				           static_cast<unsigned>( TableType::zero_char );
				int const sign = [&] {
					switch( rng.front( ) ) {
					case '-':
						rng.remove_prefix( );
						return -1;
					case '+':
						rng.remove_prefix( );
						break;
					}
					return 1;
				}( );
				while( dig < 10U ) {
					result *= 10;
					result += static_cast<std::intmax_t>( dig );
					dig = static_cast<unsigned>( rng.pop_front( ) ) -
					      static_cast<unsigned>( TableType::zero_char );
				}
				result *= sign;
				if constexpr( TextTableColumn::range_check ==
				              NumericRangeCheck::CheckForNarrowing ) {
					return daw::narrow_cast<typename TextTableColumn::parse_to>( result );
				} else {
					return static_cast<typename TextTableColumn::parse_to>( result );
				}
			}
		};

		struct Date {
			using i_am_a_text_table_parser_type = void;
		};

		struct Custom {
			using i_am_a_text_table_parser_type = void;
		};

		struct Ignored {
			using i_am_a_text_table_parser_type = void;
		};
	} // namespace TextTableParserTypes

	template<typename Number>
	using get_numeric_column_type = std::conditional_t<
	  std::is_floating_point_v<Number>, TextTableParserTypes::Real,
	  std::conditional_t<std::is_signed_v<Number>, TextTableParserTypes::Signed,
	                     std::conditional_t<std::is_unsigned_v<Number>,
	                                        TextTableParserTypes::Unsigned,
	                                        unknown_numeric_column_type>>>;

} // namespace daw::text_data::text_table_details
