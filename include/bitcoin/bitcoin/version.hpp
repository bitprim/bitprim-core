///////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2014-2015 libbitcoin developers (see COPYING).
//
//        GENERATED SOURCE CODE, DO NOT EDIT EXCEPT EXPERIMENTALLY
//
///////////////////////////////////////////////////////////////////////////////
#ifndef LIBBITCOIN_VERSION_HPP
#define LIBBITCOIN_VERSION_HPP

/**
 * The semantic version of this repository as: [major].[minor].[patch]
 * For interpretation of the versioning scheme see: http://semver.org
 */

// #define LIBBITCOIN_VERSION "3.4.0"
#define LIBBITCOIN_MAJOR_VERSION 0
#define LIBBITCOIN_MINOR_VERSION 11
#define LIBBITCOIN_PATCH_VERSION 0

// #define STR_HELPER(x) #x
// #define STR(x) STR_HELPER(x)
// #define LIBBITCOIN_VERSION STR(LIBBITCOIN_MAJOR_VERSION) "." STR(LIBBITCOIN_MINOR_VERSION) "." STR(LIBBITCOIN_PATCH_VERSION)
// #undef STR
// #undef STR_HELPER

#define LIBBITCOIN_VERSION "0.11.0"

#ifdef BITPRIM_BUILD_NUMBER
#define BITPRIM_CORE_VERSION BITPRIM_BUILD_NUMBER
#else
#define BITPRIM_CORE_VERSION "v0.0.0"
#endif

// #pragma message("LIBBITCOIN_VERSION")
// #pragma message(LIBBITCOIN_VERSION)
// #pragma message("BITPRIM_BUILD_NUMBER")
// #pragma message(BITPRIM_BUILD_NUMBER)

namespace libbitcoin {
char const* version();
} /*namespace libbitcoin*/
 

#endif
