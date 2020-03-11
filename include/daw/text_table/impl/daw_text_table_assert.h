// The MIT License (MIT)
//
// Copyright (c) 2019-2020 Darrell Wright
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

#include <cstdio>
#include <cstdlib>
#include <memory>
#include <string>
#include <string_view>

#pragma once

#if defined( __GNUC__ ) or defined( __clang__ )
#define DAW_LIKELY( Bool ) ( __builtin_expect( !!( Bool ), 1 ) )
#define DAW_UNLIKELY( Bool ) ( __builtin_expect( !!( Bool ), 0 ) )
#else
#define DAW_LIKELY( Bool ) Bool
#define DAW_UNLIKELY( Bool ) Bool
#endif

#if not( defined( __cpp_exceptions ) or defined( __EXCEPTIONS ) or             \
         defined( _CPPUNWIND ) )
// account for no exceptions -fno-exceptions
#ifdef DAW_USE_TextTable_EXCEPTIONS
#undef DAW_USE_TextTable_EXCEPTIONS
#endif
#endif

namespace daw::text_data {
	class text_table_exception {
		std::string m_reason{};

	public:
		[[maybe_unused]] text_table_exception( ) = default;
		[[maybe_unused]] inline text_table_exception(
		  std::string_view reason ) noexcept
		  : m_reason( reason ) {}

		[[nodiscard, maybe_unused]] std::string const &reason( ) const noexcept {
			return m_reason;
		}
	};
} // namespace daw::text_data

#ifdef DAW_USE_TextTable_EXCEPTIONS
inline constexpr bool use_daw_text_table_exceptions_v = true;
#else
inline constexpr bool use_daw_text_table_exceptions_v = false;
#endif

template<bool ShouldThrow = use_daw_text_table_exceptions_v>
[[maybe_unused, noreturn]] void
daw_text_table_error( std::string_view reason ) {
#ifdef DAW_USE_TextTable_EXCEPTIONS
	if constexpr( ShouldThrow ) {
		throw daw::text_data::text_table_exception( reason );
	} else {
#endif
		(void)ShouldThrow;
		(void)reason;
		std::abort( );
#ifdef DAW_USE_TextTable_EXCEPTIONS
	}
#endif
}

#ifndef DAW_TextTable_CHECK_DEBUG_ONLY
template<typename Bool>
static constexpr void daw_text_table_assert( Bool const &b,
                                             std::string_view reason ) {
	if( DAW_UNLIKELY( not static_cast<bool>( b ) ) ) {
		daw_text_table_error( reason );
	}
}

#else // undef DAW_TextTable_CHECK_DEBUG_ONLY
#ifndef NDEBUG
template<typename Bool>
static constexpr void daw_text_table_assert( Bool const &b,
                                             std::string_view reason ) {
	if( DAW_UNLIKELY( not static_cast<bool>( b ) ) ) {
		daw_text_table_error( reason );
	}
}

#else // NDEBUG set
#define daw_text_table_assert( ... )                                           \
	do {                                                                         \
	} while( false )

#endif
#endif
#undef DAW_UNLIKELY
#undef DAW_LIKELY
