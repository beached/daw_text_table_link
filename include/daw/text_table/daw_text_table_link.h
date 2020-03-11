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
#include "impl/daw_text_table_link_parsers.h"

#include <daw/daw_string_view.h>
#include <daw/daw_utility.h>

#include <cstddef>
#include <cstdint>
#include <vector>

namespace daw::text_data {
	template<COLUMNNAMETYPE Name, typename T = double,
	         NumericRangeCheck RangeCheck = NumericRangeCheck::Never,
	         typename Constructor = daw::construct_a_t<T>>
	struct text_number {
		using i_am_a_text_table_column = void;
		static constexpr daw::string_view name = Name;
		static constexpr NumericRangeCheck range_check = RangeCheck;
		using column_type = text_table_details::get_numeric_column_type<T>;
		using parse_to = T;
		using constructor = Constructor;
	};

	template<COLUMNNAMETYPE Name, typename T, typename FromConverter,
	         typename ToConverter>
	struct text_custom {
		using i_am_a_text_table_column = void;
		static constexpr daw::string_view name = Name;
		using from_converter = FromConverter;
		using to_converter = ToConverter;
		using column_type = text_table_details::TextTableParserTypes::Custom;
		using parse_to = T;
	};

	template<COLUMNNAMETYPE Name, typename T = std::string,
	         typename Constructor = daw::construct_a_t<T>,
	         typename Appender = text_table_details::basic_appender<T>>
	struct text_string {
		using i_am_a_text_table_column = void;
		static constexpr daw::string_view name = Name;
		using column_type = text_table_details::TextTableParserTypes::String;
		using parse_to = T;
		using constructor = Constructor;
	};

	template<COLUMNNAMETYPE Name, typename T = std::string_view,
	         typename Constructor = daw::construct_a_t<T>>
	struct text_string_raw {
		using i_am_a_text_table_column = void;
		static constexpr daw::string_view name = Name;
		using column_type = text_table_details::TextTableParserTypes::StringRaw;
		using parse_to = T;
		using constructor = Constructor;
	};

	template<COLUMNNAMETYPE Name>
	struct text_table_ignored {
		using i_am_a_text_table_column = void;
		// TODO: future
		daw::string_view name = Name;
		using column_type = text_table_details::TextTableParserTypes::Ignored;
	};

	template<typename... TextTableColumns>
	struct text_column_list {
		template<typename TableType>
		using location_type =
		  text_table_details::locations_info_t<typename TableType::CharT,
		                                       TextTableColumns...>;

		template<typename TableType>
		[[nodiscard]] static constexpr location_type<TableType>
		location_info( TableState<TableType> &state ) {
			return text_table_details::fill_location_info<TextTableColumns...>(
			  state );
		}

		template<typename T, typename TableType, typename CharT>
		[[nodiscard]] static constexpr T
		parse_row( TableState<TableType> &state,
		           text_table_details::locations_info_t<CharT, TextTableColumns...>
		             &loc_info ) {
			return text_table_details::parse_table_row<T, TextTableColumns...>(
			  state, loc_info, std::index_sequence_for<TextTableColumns...>{} );
		}
	};

	template<typename T, typename Container, typename Constructor,
	         typename Appender, typename CharT>
	[[maybe_unused, nodiscard]] constexpr Container
	parse_csv_table_impl( daw::basic_string_view<CharT> rng ) {
		using table_type = basic_csv_table_type<CharT>;
		using parser_t = text_table_details::text_table_data_contract_trait_t<T>;

		auto state = TableState<table_type>( rng );
		auto loc_info = parser_t::template location_info<table_type>( state );

		auto result = Constructor{}( );
		auto appender = Appender( result );

		while( not state.at_eof( ) ) {
			appender( parser_t::template parse_row<T>( state, loc_info ) );
		}
		return result;
	}

	template<typename T, typename Container = std::vector<T>,
	         typename Constructor = daw::construct_a_t<Container>,
	         typename Appender = text_table_details::basic_appender<Container>>
	[[maybe_unused, nodiscard]] constexpr Container
	parse_csv_table( std::basic_string_view<char> rng ) {
		return parse_csv_table_impl<T, Container, Constructor, Appender>(
		  daw::basic_string_view<char>( rng.data( ), rng.size( ) ) );
	}

	template<typename T, typename Container = std::vector<T>,
	         typename Constructor = daw::construct_a_t<Container>,
	         typename Appender = text_table_details::basic_appender<Container>>
	[[maybe_unused, nodiscard]] constexpr Container
	parse_csv_table( std::basic_string_view<wchar_t> rng ) {
		return parse_csv_table_impl<T, Container, Constructor, Appender>(
		  daw::basic_string_view<wchar_t>( rng.data( ), rng.size( ) ) );
	}
} // namespace daw::text_data
