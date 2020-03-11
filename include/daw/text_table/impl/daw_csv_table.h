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

#include "daw_text_table_link_common.h"
#include "daw_text_table_link_parsers.h"

#include <daw/daw_string_view.h>
#include <daw/daw_utility.h>

#include <cstdint>
#include <limits>

namespace daw::text_data {
	inline static constexpr std::size_t NoHeaderRow =
	  std::numeric_limits<std::size_t>::max( );

	template<typename CharType, std::size_t HeaderRow = 0,
	         std::size_t DataRow = HeaderRow + 1U,
	         bool SkipLeadingWhiteSpace = false>
	struct basic_csv_table_type {
		static_assert( NoHeaderRow or DataRow > HeaderRow,
		               "Header Row must preceed data" );

		using i_am_a_table_type = void;
		using CharT = CharType;
		static constexpr CharT delimiter_char = static_cast<CharT>( ',' );
		static constexpr CharT quote_char = static_cast<CharT>( '"' );
		static constexpr CharT zero_char = static_cast<CharT>( '0' );
		static constexpr CharT newline_char = static_cast<CharT>( '\n' );
		static constexpr CharT escape_char = static_cast<CharT>( '\\' );
		static constexpr bool has_header = HeaderRow != NoHeaderRow;

		template<typename CharRange>
		static constexpr void row_move_to_next( CharRange &rng ) {
			bool is_escaped = false;
			bool in_quote = false;
			auto const sz = rng.size( );

			for( std::size_t n = 0; n < sz; ++n ) {
				if( is_escaped ) {
					is_escaped = false;
					continue;
				}
				auto const c = rng[n];
				if( c == escape_char ) {
					is_escaped = true;
					continue;
				}
				if( c == quote_char ) {
					if( n > 0 and rng[n - 1] == quote_char ) {
						continue;
					}
					in_quote = not in_quote;
					continue;
				}
				if( c == newline_char ) {
					rng.remove_prefix( n + 1 );
					return;
				}
			}
		}

		template<typename CharRange>
		static constexpr std::size_t row_move_to_header( CharRange &rng ) {
			// Assumes that there is no escaping prior to header data
			for( size_t n = 0; n < HeaderRow; ++n ) {
				(void)rng.pop_front( {&newline_char, 1} );
			}
			return HeaderRow;
		}

		template<typename CharRange>
		static constexpr std::size_t row_move_to_data( CharRange &rng ) {
			auto skip_rows = has_header ? DataRow - HeaderRow : DataRow;
			daw_text_table_assert( skip_rows == 0 or not rng.empty( ),
			                       "Unexpected end of data" );
			while( skip_rows-- > 0 ) {
				if constexpr( has_header ) {
					row_move_to_next( rng );
				} else {
					(void)rng.pop_front( {&newline_char, 1} );
				}
			}
			return DataRow;
		}

		template<typename CharRange>
		static constexpr CharRange column_get_next( CharRange &rng ) {
			daw_text_table_assert( not rng.empty( ), "Unexpected end of data" );

			auto first = rng.begin( );
			if constexpr( SkipLeadingWhiteSpace ) {
				trim_left( rng );
			}
			if( rng.empty( ) ) {
				return CharRange( first,
				                  static_cast<std::size_t>( rng.begin( ) - first ) );
			}
			if( rng.front( ) == quote_char ) {
				return find_end_of_quoted_cell( first, rng );
			}
			return find_end_of_unquoted_cell( first, rng );
		}

	private:
		template<typename First, typename CharRange>
		static constexpr CharRange find_end_of_unquoted_cell( First first,
		                                                      CharRange &rng ) {
			(void)rng.pop_front( [is_escaped = false]( CharT c ) mutable {
				if( is_escaped ) {
					is_escaped = false;
					return false;
				}
				is_escaped = c == escape_char;
				if constexpr( SkipLeadingWhiteSpace ) {
					return daw::parser::is_unicode_whitespace( c ) or
					       c == delimiter_char or c == newline_char;
				} else {
					return c == delimiter_char or c == newline_char;
				}
			} );
			auto result =
			  CharRange( first, static_cast<std::size_t>( rng.begin( ) - first ) );
			trim_left( rng );
			if( not rng.empty( ) and rng.front( ) == delimiter_char ) {
				result.remove_suffix( );
			}
			return result;
		}

		template<typename First, typename CharRange>
		static constexpr CharRange find_end_of_quoted_cell( First first,
		                                                    CharRange &rng ) {
			rng.remove_prefix( );
			first = rng.begin( );
			while( not rng.empty( ) ) {
				if( rng.front( ) == quote_char ) {
					if( rng.size( ) > 1 and rng[1] == quote_char ) {
						// Escaped Quote
						rng.remove_prefix( );
					} else {
						break;
					}
				}
				rng.remove_prefix( );
			}
			auto result =
			  CharRange( first, static_cast<std::size_t>( rng.begin( ) - first ) );
			rng.remove_prefix( );
			column_move_to_next( rng );
			return result;
		}

		template<typename CharRange>
		static constexpr void trim_left( CharRange &rng ) {
			auto pos = rng.find_first_of_if( []( CharT c ) {
				return c == newline_char or not daw::parser::is_unicode_whitespace( c );
			} );
			if( pos > 0 ) {
				rng.remove_prefix( pos );
			}
		}

		template<typename CharRange>
		static constexpr void column_move_to_next( CharRange &rng ) {
			trim_left( rng );
			daw_text_table_assert( rng.front( ) == delimiter_char or
			                         rng.front( ) == newline_char,
			                       "Expected next column or new row" );
			if( not( rng.empty( ) or rng.front( ) == newline_char ) ) {
				rng.remove_prefix( );
			}
		}
	};
} // namespace daw::text_data
