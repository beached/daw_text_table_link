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

#include "daw/text_table/daw_text_table_iterator.h"
#include "daw/text_table/daw_text_table_link.h"

#include <daw/daw_benchmark.h>
#include <daw/daw_memory_mapped_file.h>

#include <cstdio>
#include <iterator>
#include <string>

struct world_cities_pop {
	std::string_view country;
	std::string_view city;
	std::string_view accentcity;
	std::string_view region;
	std::string_view population;
	std::string_view latitude;
	std::string_view longitude;
};

struct empty {};

namespace daw::text_data {
	template<>
	struct text_data_contract<world_cities_pop> {
		static constexpr char const country[] = "Country";
		static constexpr char const city[] = "City";
		static constexpr char const accentcity[] = "AccentCity";
		static constexpr char const region[] = "Region";
		static constexpr char const population[] = "Population";
		static constexpr char const latitude[] = "Latitude";
		static constexpr char const longitude[] = "Longitude";
		using type =
		  text_column_list<text_string_raw<country>, text_string_raw<city>,
		                   text_string_raw<accentcity>, text_string_raw<region>,
		                   text_string_raw<population>, text_string_raw<latitude>,
		                   text_string_raw<longitude>>;
	};

	template<>
	struct text_data_contract<empty> {
		using type = text_column_list<>;
	};
} // namespace daw::text_data

int main( int argc, char **argv ) {
	if( argc <= 1 ) {
		puts( "Must supply path to worldcitiespop.txt\n" );
		exit( EXIT_FAILURE );
	}
	auto data = daw::filesystem::memory_mapped_file_t<>( argv[1] );

	using iter_t = daw::text_data::csv_table_iterator<world_cities_pop>;
	using iter2_t = daw::text_data::csv_table_iterator<empty>;
	auto first = iter_t( {data.data( ), data.size( )} );
	auto first2 = iter2_t( {data.data( ), data.size( )} );
	static constexpr auto last = iter_t( );
	static constexpr auto last2 = iter2_t( );

#ifdef NDEBUG
	static constexpr std::size_t num_runs = 10;
#else
	static constexpr std::size_t num_runs = 1;
#endif
	daw::bench_n_test_mbs<num_runs>(
	  "world cities population", data.size( ),
	  []( iter_t f ) {
		  while( f != last ) {
			  daw::do_not_optimize( *f );
			  ++f;
		  }
	  },
	  first );

	std::size_t row_count = 0;
	daw::bench_n_test_mbs<num_runs>(
	  "row_count", data.size( ),
	  [&row_count]( iter2_t f ) {
		  row_count = static_cast<std::size_t>( std::distance( f, last2 ) );
	  },
	  first2 );
	daw::do_not_optimize( row_count );
}
