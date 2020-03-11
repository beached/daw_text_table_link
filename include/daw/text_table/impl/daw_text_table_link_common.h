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
#include "daw_text_table_link_table_state.h"

#include <daw/cpp_17.h>
#include <daw/daw_algorithm.h>
#include <daw/daw_parser_helper_sv.h>
#include <daw/daw_string_view.h>
#include <daw/daw_utility.h>

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cwchar>
#include <type_traits>
#include <utility>

namespace daw::text_data {
#if defined( __cpp_nontype_template_parameter_class )
	// C++ 20 Non-Type Class Template Arguments

	/**
	 * A fixed string used for member names in text_data descriptions
	 * @tparam N size of string plus 1.  Do not set explicitly.  Use CTAD
	 */
	template<std::size_t N>
	struct column_name {
		static_assert( N > 0 );
		char const m_data[N]{};

	private:
		template<std::size_t... Is>
		constexpr column_name( char const ( &ptr )[N], std::index_sequence<Is...> )
		  : m_data{ptr[Is]...} {}

	public:
		constexpr column_name( char const ( &ptr )[N] )
		  : column_name( ptr, std::make_index_sequence<N>{} ) {}

		constexpr operator daw::string_view( ) const {
			return {m_data, N - 1};
		}

		// Needed for copy_to_iterator
		[[nodiscard]] constexpr char const *begin( ) const {
			return m_data;
		}

		// Needed for copy_to_iterator
		[[nodiscard]] constexpr char const *end( ) const {
			return m_data + static_cast<ptrdiff_t>( size( ) );
		}

		[[nodiscard]] static constexpr std::size_t size( ) noexcept {
			return N - 1;
		}

		template<std::size_t M>
		constexpr bool operator==( column_name<M> const &rhs ) const {
			if( N != M ) {
				return false;
			}
			for( std::size_t n = 0; n < N; ++n ) {
				if( m_data[n] != rhs.m_data[n] ) {
					return false;
				}
			}
			return true;
		}

		constexpr bool operator==( daw::string_view sv ) const {
			return daw::string_view( m_data, N - 1 ) == sv;
		}

		constexpr operator std::string_view( ) const {
			return std::string_view( m_data, N - 1 );
		}
	};

	template<typename... Chars>
	column_name( Chars... )->column_name<sizeof...( Chars )>;

#define COLUMNNAMETYPE daw::text_data::column_name

	// Convienience for array members that are required to be unnamed
	inline constexpr COLUMNNAMETYPE no_name{""};

	namespace text_table_details {
		template<COLUMNNAMETYPE n>
		inline constexpr bool is_no_name = ( n == no_name );

		template<typename Name>
		constexpr char const *as_cstr( Name const &n ) {
			return n.begin( );
		}
	} // namespace text_table_details
#else
#define COLUMNNAMETYPE char const *
	// Convienience for array members that are required to be unnamed
	inline constexpr char const no_name[] = "";

	namespace text_table_details {
		template<COLUMNNAMETYPE n>
		inline constexpr bool
		  is_no_name = daw::string_view( n ) == daw::string_view( "" );

		constexpr char const *as_cstr( COLUMNNAMETYPE ptr ) {
			return ptr;
		}
	} // namespace text_table_details
#endif
	/**
	 * Allows having literals parse that are encoded as strings. It allows
	 * one to have it be Never true, Maybe true or Always true.  This controls
	 * whether the parser will Never remove quotes, check if quotes exist, or
	 * Always remove quotes around the literal
	 */
	enum class QuotingOptions : std::uint8_t { Never, Maybe, Always };

	enum class NumericRangeCheck : bool {
		Never = false,
		CheckForNarrowing = true
	};

	namespace text_table_details {
		/***
		 * Attempt to parse/serialize a type that has not yet been mapped
		 */
		template<typename>
		struct missing_text_table_data_contract_for {};
	} // namespace text_table_details

	/***
	 * Mapping class for CSB data structures to C++
	 * @tparam T Class to map
	 */
	template<typename T>
	struct text_data_contract {
		using type = text_table_details::missing_text_table_data_contract_for<T>;
	};

	namespace text_table_details {
		template<typename T>
		using text_table_data_contract_trait_t =
		  typename daw::text_data::text_data_contract<T>::type;

		template<typename T>
		struct has_text_table_data_contract_trait
		  : std::bool_constant<
		      not std::is_same_v<missing_text_table_data_contract_for<T>,
		                         text_table_data_contract_trait_t<T>>> {};

		template<typename T>
		inline constexpr bool has_text_table_data_contract_trait_v =
		  has_text_table_data_contract_trait<T>::value;

		template<typename Container, typename Value>
		using detect_push_back = decltype(
		  std::declval<Container &>( ).push_back( std::declval<Value>( ) ) );

		template<typename Container, typename Value>
		using detect_insert_end = decltype( std::declval<Container &>( ).insert(
		  std::end( std::declval<Container &>( ) ), std::declval<Value>( ) ) );

		template<typename Container, typename Value>
		inline constexpr bool has_push_back_v =
		  daw::is_detected_v<detect_push_back, Container, Value>;

		template<typename Container, typename Value>
		inline constexpr bool has_insert_end_v =
		  daw::is_detected_v<detect_insert_end, Container, Value>;

		template<typename Container>
		struct basic_appender {
			Container *m_container;

			explicit constexpr basic_appender( Container &container )
			  : m_container( &container ) {}

			template<typename Value>
			constexpr void operator( )( Value &&value ) const {
				if constexpr( has_push_back_v<Container, daw::remove_cvref_t<Value>> ) {
					m_container->push_back( std::forward<Value>( value ) );
				} else if constexpr( has_insert_end_v<Container,
				                                      daw::remove_cvref_t<Value>> ) {
					m_container->insert( std::end( *m_container ),
					                     std::forward<Value>( value ) );
				} else {
					static_assert(
					  has_push_back_v<Container, daw::remove_cvref_t<Value>> or
					    has_insert_end_v<Container, daw::remove_cvref_t<Value>>,
					  "basic_appender requires a Container that either has push_back or "
					  "insert with the end iterator as first argument" );
				}
			}

			template<typename Value,
			         std::enable_if_t<
			           not std::is_same_v<basic_appender, daw::remove_cvref_t<Value>>,
			           std::nullptr_t> = nullptr>
			basic_appender &operator=( Value &&v ) {
				operator( )( std::forward<Value>( v ) );
				return *this;
			}

			basic_appender &operator++( ) {
				return *this;
			}

			basic_appender operator++( int ) {
				return *this;
			}

			basic_appender &operator*( ) {
				return *this;
			}
		};

		struct unknown_numeric_column_type {};

		template<typename T>
		using is_text_table_parser_test = typename T::i_am_a_text_table_parser_type;

		template<typename T>
		inline constexpr bool is_text_table_parser_v =
		  daw::is_detected_v<is_text_table_parser_test, T>;

		template<typename CharT>
		struct location_info_t {
			daw::basic_string_view<CharT> name;
			daw::basic_string_view<CharT> location{};
			bool found = false;
			size_t column = std::numeric_limits<std::size_t>::max( );

			constexpr location_info_t( daw::string_view n )
			  : name( n ) {}
		};

		template<typename, typename T>
		constexpr T always_value( T &&val ) {
			return val;
		}

		template<typename CharT, typename... TextTableColumns>
		struct locations_info_t {
			std::array<location_info_t<CharT>, sizeof...( TextTableColumns )>
			  locations;

			constexpr location_info_t<CharT> &operator[]( std::size_t idx ) {
				return locations[idx];
			}

			constexpr location_info_t<CharT> const &
			operator[]( std::size_t idx ) const {
				return locations[idx];
			}

			constexpr std::optional<std::size_t>
			find_name( daw::basic_string_view<CharT> name ) const {
				auto pos = daw::algorithm::find_if(
				  locations.begin( ), locations.end( ),
				  [name]( location_info_t<CharT> const &item ) {
					  return name == item.name;
				  } );
				if( pos == locations.end( ) ) {
					return {};
				}
				return static_cast<std::size_t>(
				  std::distance( locations.begin( ), pos ) );
			}

			constexpr std::optional<std::size_t> find_col( std::size_t idx ) const {
				auto pos = std::find_if( locations.begin( ), locations.end( ),
				                         [idx]( location_info_t<CharT> const &item ) {
					                         return idx == item.column;
				                         } );
				if( pos == locations.end( ) ) {
					return {};
				}
				return static_cast<std::size_t>(
				  std::distance( locations.begin( ), pos ) );
			}
		};

		template<typename CharT, typename... TextTableColumns>
		inline constexpr locations_info_t<CharT, TextTableColumns...>
		  locations_info = {location_info_t<CharT>( TextTableColumns::name )...};

		template<typename TextTableColumn>
		using column_name_type = decltype( TextTableColumn::name );

		template<typename TextTableColumn>
		inline constexpr bool column_requires_header =
		  daw::is_detected_v<column_name_type, TextTableColumn>;

		template<typename... TextTableColumns>
		inline constexpr bool columns_require_header =
		  ( column_requires_header<TextTableColumns> or ... );

		template<typename... TextTableColumns, typename TableType>
		[[maybe_unused,
		  nodiscard]] constexpr locations_info_t<typename TableType::CharT,
		                                         TextTableColumns...>
		fill_location_info( TableState<TableType> &state ) {
			static_assert( TableType::has_header or
			                 not columns_require_header<TextTableColumns...>,
			               "A valid header row is required for named columns" );
			auto known_locations =
			  locations_info<typename TableType::CharT, TextTableColumns...>;
			state.row_move_to_header( );
			if constexpr( sizeof...( TextTableColumns ) > 0 ) {
				daw_text_table_assert( not state.at_eol( ), "Expected column headers" );
				std::size_t col = 0;
				std::size_t found_count = 0;
				while( found_count < sizeof...( TextTableColumns ) and
				       not state.at_eol( ) ) {
					auto name = state.column_get_next( );
					if( auto pos = known_locations.find_name( name ); pos ) {
						known_locations.locations[*pos].column = col;
						++found_count;
					}
					++col;
				}
				daw_text_table_assert( found_count == sizeof...( TextTableColumns ),
				                       "Could not find all mapped columns" );
			}
			state.row_move_to_data( );
			return known_locations;
		}

		template<std::size_t N, typename TableType, typename... TextTableColumns>
		constexpr auto
		find_cell( TableState<TableType> &state,
		           locations_info_t<typename TableType::CharT, TextTableColumns...>
		             &loc_info ) {
			if( loc_info[N].found ) {
				return loc_info[N].location;
			}

			auto result = state.column_get_next( );
			while( state.col( ) < loc_info.locations[N].column ) {
				if( auto const cell_idx = loc_info.find_col( state.col( ) );
				    cell_idx ) {
					loc_info[*cell_idx].location = result;
					loc_info[*cell_idx].found = true;
				}
				result = state.column_get_next( );
			}
			return result;
		}

		template<typename TextTableColumn, std::size_t N, typename LocationInfo,
		         typename TableType>
		constexpr typename TextTableColumn::parse_to
		parse_cell( TableState<TableType> &state, LocationInfo &loc_info ) {
			using parse_tag = typename TextTableColumn::column_type;
			return parse_tag::template parse_value<TextTableColumn, TableType>(
			  find_cell<N, TableType>( state, loc_info ) );
		}

		template<typename T, typename... TextTableColumns, std::size_t... Is,
		         typename TableType>
		constexpr T
		parse_table_row( TableState<TableType> &state,
		                 locations_info_t<typename TableType::CharT,
		                                  TextTableColumns...> &loc_info,
		                 std::index_sequence<Is...> ) {

			for( auto &item : loc_info.locations ) {
				item.found = false;
			}
			using tp_t = std::tuple<decltype(
			  parse_cell<TextTableColumns, Is>( state, loc_info ) )...>;
			// TODO use OnExit to get guaranteed copy elision
			auto result = std::apply(
			  daw::construct_a_t<T>{},
			  tp_t{parse_cell<TextTableColumns, Is>( state, loc_info )...} );
			state.row_move_to_next( );
			return result;
		}
	} // namespace text_table_details
} // namespace daw::text_data
