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

#include <cstdlib>
#include <cwchar>
#include <type_traits>

namespace daw::text_data::text_table_details {
	template<typename Real>
	auto str_to_real( char const *first, char **last ) {
		static_assert( std::is_floating_point_v<Real> );
		if constexpr( std::is_same_v<Real, float> ) {
			return std::strtof( first, last );
		} else if constexpr( std::is_same_v<Real, double> ) {
			return std::strtod( first, last );
		} else if constexpr( std::is_same_v<Real, long double> ) {
			return std::strtold( first, last );
		} else {
			static_assert( std::disjunction_v<std::is_same<Real, float>,
			                                  std::is_same<Real, double>,
			                                  std::is_same<Real, long double>>,
			               "Unknown floating point type" );
		}
	}

	template<typename Real>
	auto str_to_real( wchar_t const *first, wchar_t **last ) {
		static_assert( std::is_floating_point_v<Real> );
		if constexpr( std::is_same_v<Real, float> ) {
			return std::wcstof( first, last );
		} else if constexpr( std::is_same_v<Real, double> ) {
			return std::wcstod( first, last );
		} else if constexpr( std::is_same_v<Real, long double> ) {
			return std::wcstold( first, last );
		} else {
			static_assert( std::disjunction_v<std::is_same<Real, float>,
			                                  std::is_same<Real, double>,
			                                  std::is_same<Real, long double>>,
			               "Unknown floating point type" );
		}
	}
} // namespace daw::text_data::text_table_details
