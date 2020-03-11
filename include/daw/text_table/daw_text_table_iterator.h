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

#include "impl/daw_csv_table.h"
#include "impl/daw_text_table_link_common.h"
#include "impl/daw_text_table_link_table_state.h"

#include <optional>
#include <utility>

namespace daw::text_data {
	namespace text_data_details {
		template<typename T>
		struct arrow_proxy {
			T value;

			[[nodiscard]] constexpr T *operator->( ) {
				return &value;
			}
		};
	} // namespace text_data_details

	template<typename T, typename TableType>
	struct basic_text_table_iterator {
		using value_type = T;
		using reference = value_type;
		using pointer = text_data_details::arrow_proxy<value_type>;
		using difference_type = std::ptrdiff_t;
		using iterator_category = std::forward_iterator_tag;
		using CharT = typename TableType::CharT;

	private:
		using parser_t = text_table_details::text_table_data_contract_trait_t<T>;
		using location_type = typename parser_t::template location_type<TableType>;

		TableState<TableType> m_state{daw::basic_string_view<CharT>( )};
		std::optional<location_type> m_loc_info{};
		std::optional<TableState<TableType>> m_last_state{};

	public:
		constexpr basic_text_table_iterator( ) = default;
		constexpr basic_text_table_iterator( std::basic_string_view<CharT> data )
		  : m_state( TableState<TableType>(
		      daw::basic_string_view<CharT>( data.data( ), data.size( ) ) ) )
		  , m_loc_info( parser_t::location_info( m_state ) ) {}

		constexpr value_type operator*( ) {
			if( not m_last_state ) {
				m_last_state = std::optional<TableState<TableType>>( m_state );
				return parser_t::template parse_row<T>( m_state, *m_loc_info );
			}
			m_state = *m_last_state;
			return parser_t::template parse_row<T>( m_state, *m_loc_info );
		}

		constexpr pointer operator->( ) {
			return pointer{operator*( )};
		}

		constexpr basic_text_table_iterator &operator++( ) {
			m_state.row_move_to_next( );
			m_last_state = std::optional<TableState<TableType>>( );
			return *this;
		}

		constexpr basic_text_table_iterator operator++( int ) {
			auto result = *this;
			(void)operator++( );
			return result;
		}

		[[nodiscard]] explicit constexpr operator bool( ) const {
			return m_state.at_eof( );
		}

		constexpr bool operator==( basic_text_table_iterator const &rhs ) const {
			return m_state == rhs.m_state;
		}

		constexpr bool operator!=( basic_text_table_iterator const &rhs ) const {
			return m_state != rhs.m_state;
		}
	};

	template<typename T, typename CharT = char, std::size_t HeaderRow = 0,
	         std::size_t DataRow = HeaderRow + 1U,
	         bool SkipLeadingWhiteSpace = false>
	using csv_table_iterator = basic_text_table_iterator<
	  T, basic_csv_table_type<CharT, HeaderRow, DataRow, SkipLeadingWhiteSpace>>;
} // namespace daw::text_data
