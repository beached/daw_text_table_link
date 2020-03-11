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

#include <string>

struct test_001 {
	int n;
	std::string s;
};

namespace daw::text_data {
	template<>
	struct text_data_contract<test_001> {
		static constexpr char const a[] = "a";
		static constexpr char const s[] = "s";

		using type = text_column_list<text_number<a, int>, text_string<s>>;
	};
} // namespace daw::text_data

constexpr char const text_table0[] = R"("a","s",d
5,  hello, 33
1,"bye", 44
)";

int main( ) {
	auto tbl = daw::text_data::parse_csv_table<test_001>( text_table0 );
	auto v0 = tbl[0].n + tbl[1].n;

	using iter_t = daw::text_data::csv_table_iterator<test_001>;
	auto first = iter_t( text_table0 );
	constexpr auto last = iter_t( );
	int v1 = 0;
	while( first != last ) {
		v1 += first->n;
		++first;
	}
	daw_text_table_assert( v0 == v1, "Expected same" );
}
