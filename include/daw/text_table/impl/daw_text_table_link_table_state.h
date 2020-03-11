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

#include <daw/cpp_17.h>
#include <daw/daw_string_view.h>
#include <daw/daw_utility.h>

#include <cstdint>

namespace daw::text_data {
	namespace text_table_details {
		template<typename T>
		using is_a_table_type_test = typename T::i_am_a_table_type;

		template<typename T>
		inline constexpr bool is_a_table_type_v =
		  daw::is_detected_v<is_a_table_type_test, T>;
	} // namespace text_table_details

	template<typename TableType>
	struct TableState {
		static_assert(
		  text_table_details::is_a_table_type_v<TableType>,
		  "The TableType has not declared itself to be a valid TableType" );

		using CharT = typename TableType::CharT;
		static constexpr CharT delimiter_char = TableType::delimiter_char;
		static constexpr CharT quote_char = TableType::quote_char;
		static constexpr CharT zero_char = TableType::zero_char;
		static constexpr CharT newline_char = TableType::newline_char;
		static constexpr CharT escape_char = TableType::escape_char;
		static constexpr bool has_header = TableType::has_header;

	private:
		daw::basic_string_view<CharT> m_state;
		std::size_t m_col = 0;
		std::size_t m_row = 0;

	public:
		constexpr TableState( daw::basic_string_view<CharT> table_data )
		  : m_state( table_data ) {}

		constexpr void row_move_to_next( ) {
			m_col = 0;
			++m_row;
			TableType::row_move_to_next( m_state );
		}

		constexpr void row_move_to_header( ) {
			m_col = 0;
			m_row = TableType::row_move_to_header( m_state );
		}

		constexpr void row_move_to_data( ) {
			m_col = 0;
			m_row = TableType::row_move_to_data( m_state );
		}

		constexpr daw::basic_string_view<CharT> column_get_next( ) {
			++m_col;
			return TableType::column_get_next( m_state );
		}

		constexpr bool at_eol( ) const {
			return m_state.empty( ) or m_state.front( ) == newline_char;
		}

		constexpr bool at_eof( ) const {
			return m_state.empty( );
		}

		constexpr std::size_t col( ) const {
			return m_col;
		}

		constexpr bool operator==( TableState const &rhs ) const {
			return ( at_eof( ) and rhs.at_eof( ) ) or
			       m_state.data( ) == rhs.m_state.data( );
		}

		constexpr bool operator!=( TableState const &rhs ) const {
			return not operator==( rhs );
		}
	};
} // namespace daw::text_data
