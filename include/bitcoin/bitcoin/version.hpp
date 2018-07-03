///////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2014-2015 libbitcoin developers (see COPYING).
//
//        GENERATED SOURCE CODE, DO NOT EDIT EXCEPT EXPERIMENTALLY
//
///////////////////////////////////////////////////////////////////////////////
#ifndef LIBBITCOIN_VERSION_HPP_
#define LIBBITCOIN_VERSION_HPP_

/**
 * The semantic version of this repository as: [major].[minor].[patch]
 * For interpretation of the versioning scheme see: http://semver.org
 */

#ifdef BITPRIM_PROJECT_VERSION
#define BITPRIM_CORE_VERSION BITPRIM_PROJECT_VERSION
#else
#define BITPRIM_CORE_VERSION "0.0.0"
#endif

#define LIBBITCOIN_VERSION BITPRIM_CORE_VERSION

namespace libbitcoin {
char const* version();
} /*namespace libbitcoin*/
 
#endif // LIBBITCOIN_VERSION_HPP_
