/**
 * Copyright (c) 2011-2017 libbitcoin developers (see AUTHORS)
 *
 * This file is part of libbitcoin.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <bitcoin/bitcoin/wallet/payment_address.hpp>

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <string>
#include <utility>
#include <boost/program_options.hpp>
#include <bitcoin/bitcoin/formats/base_58.hpp>
#include <bitcoin/bitcoin/math/checksum.hpp>
#include <bitcoin/bitcoin/math/elliptic_curve.hpp>
#include <bitcoin/bitcoin/math/hash.hpp>
#include <bitcoin/bitcoin/multi_crypto_support.hpp>
#include <bitcoin/bitcoin/wallet/ec_private.hpp>
#include <bitcoin/bitcoin/wallet/ec_public.hpp>

#ifdef BITPRIM_CURRENCY_BCH
#include <bitcoin/bitcoin/wallet/cashaddr.hpp>
#endif //BITPRIM_CURRENCY_BCH


namespace libbitcoin {
namespace wallet {

using namespace bc::machine;

#ifdef BITPRIM_CURRENCY_LTC
const uint8_t payment_address::mainnet_p2kh = 0x30;
#else
const uint8_t payment_address::mainnet_p2kh = 0x00;
#endif

const uint8_t payment_address::mainnet_p2sh = 0x05;

const uint8_t payment_address::testnet_p2kh = 0x6f;
const uint8_t payment_address::testnet_p2sh = 0xc4;

#ifdef BITPRIM_CURRENCY_BCH
const std::string payment_address::cashaddr_prefix_mainnet = "bitcoincash";
const std::string payment_address::cashaddr_prefix_testnet = "bchtest";
#endif

payment_address::payment_address()
  : valid_(false), version_(0), hash_(null_short_hash)
{
}

payment_address::payment_address(payment_address&& other)
  : valid_(other.valid_), version_(other.version_),
    hash_(std::move(other.hash_))
{
}

payment_address::payment_address(const payment_address& other)
  : valid_(other.valid_), version_(other.version_), hash_(other.hash_)
{
}

payment_address::payment_address(const payment& decoded)
  : payment_address(from_payment(decoded))
{
}

payment_address::payment_address(const std::string& address)
  : payment_address(from_string(address))
{
}

payment_address::payment_address(const ec_private& secret)
  : payment_address(from_private(secret))
{
}

payment_address::payment_address(const ec_public& point, uint8_t version)
  : payment_address(from_public(point, version))
{
}

payment_address::payment_address(const chain::script& script, uint8_t version)
  : payment_address(from_script(script, version))
{
}

payment_address::payment_address(short_hash&& hash, uint8_t version)
  : valid_(true), version_(version), hash_(std::move(hash))
{
}

payment_address::payment_address(const short_hash& hash, uint8_t version)
  : valid_(true), version_(version), hash_(hash)
{
}

// Validators.
// ----------------------------------------------------------------------------

bool payment_address::is_address(data_slice decoded)
{
    return (decoded.size() == payment_size) && verify_checksum(decoded);
}

// Factories.
// ----------------------------------------------------------------------------
#ifdef BITPRIM_CURRENCY_BCH

template <int frombits, int tobits, bool pad, typename O, typename I>
bool ConvertBits(O &out, I it, I end) {
    size_t acc = 0;
    size_t bits = 0;
    constexpr size_t maxv = (1 << tobits) - 1;
    constexpr size_t max_acc = (1 << (frombits + tobits - 1)) - 1;
    while (it != end) {
        acc = ((acc << frombits) | *it) & max_acc;
        bits += frombits;
        while (bits >= tobits) {
            bits -= tobits;
            out.push_back((acc >> bits) & maxv);
        }
        ++it;
    }

    // We have remaining bits to encode but do not pad.
    if (!pad && bits) {
        return false;
    }

    // We have remaining bits to encode so we do pad.
    if (pad && bits) {
        out.push_back((acc << (tobits - bits)) & maxv);
    }

    return true;
}

 enum CashAddrType : uint8_t { PUBKEY_TYPE = 0, SCRIPT_TYPE = 1 };

// CashAddrContent DecodeCashAddrContent(std::string const& address) {
payment_address payment_address::from_string_cashaddr(std::string const& address) {
    // In order to avoid using the wrong network address, the from_string method
    // only accepts the cashaddr_prefix set on the multi_crypto_support file

    // TODO: validate the network on RPC/Interface calls and make payment_address independent of the network

    std::string prefix;
    data_chunk payload;
    std::tie(prefix, payload) = cashaddr::decode(address, cashaddr_prefix());

    if (prefix != cashaddr_prefix()) {
        return{};
    }

    if (payload.empty()) {
        return{};
    }

    // Check that the padding is zero.
    size_t extrabits = payload.size() * 5 % 8;
    if (extrabits >= 5) {
        // We have more padding than allowed.
        return{};
    }

    uint8_t last = payload.back();
    uint8_t mask = (1 << extrabits) - 1;
    if (last & mask) {
        // We have non zero bits as padding.
        return{};
    }

    data_chunk data;
    data.reserve(payload.size() * 5 / 8);
    ConvertBits<5, 8, false>(data, std::begin(payload), std::end(payload));

    // Decode type and size from the version.
    uint8_t version = data[0];
    if (version & 0x80) {
        // First bit is reserved.
        return{};
    }

    auto type = CashAddrType((version >> 3) & 0x1f);
    uint32_t hash_size = 20 + 4 * (version & 0x03);
    if (version & 0x04) {
        hash_size *= 2;
    }

    // Check that we decoded the exact number of bytes we expected.
    if (data.size() != hash_size + 1) {
        return{};
    }

    //uint8_t version2;
    //switch (type) {
    //    case PUBKEY_TYPE:
    //        version2 = 0x00;
    //        break;
    //    case SCRIPT_TYPE:
    //        version2 = 0x05;
    //        break;
    //}

    short_hash hash;
    std::copy(std::begin(data) + 1, std::end(data), std::begin(hash));

    if (prefix == payment_address::cashaddr_prefix_mainnet) {
        return payment_address(hash, type == PUBKEY_TYPE ? payment_address::mainnet_p2kh : payment_address::mainnet_p2sh);
    } else {
        return payment_address(hash, type == PUBKEY_TYPE ? payment_address::testnet_p2kh : payment_address::testnet_p2sh);
    }

    // // Pop the version.
    // data.erase(data.begin());
    // return {type, std::move(data)};
}

#endif //BITPRIM_CURRENCY_BCH

payment_address payment_address::from_string(const std::string& address) {
    payment decoded;
    if ( ! decode_base58(decoded, address) || !is_address(decoded)) {
#ifdef BITPRIM_CURRENCY_BCH
    return from_string_cashaddr(address);
#else
    return{};
#endif //BITPRIM_CURRENCY_BCH
    }
    return{ decoded };
}

payment_address payment_address::from_payment(const payment& decoded)
{
    if (!is_address(decoded))
        return{};

    const auto hash = slice<1, short_hash_size + 1>(decoded);
    return{ hash, decoded.front() };
}

payment_address payment_address::from_private(const ec_private& secret)
{
    if (!secret)
        return{};

    return{ secret.to_public(), secret.payment_version() };
}

payment_address payment_address::from_public(const ec_public& point,
    uint8_t version)
{
    if (!point)
        return{};

    data_chunk data;
    if (!point.to_data(data))
        return{};

    return{ bitcoin_short_hash(data), version };
}

payment_address payment_address::from_script(const chain::script& script,
    uint8_t version)
{
    // Working around VC++ CTP compiler break here.
    const auto data = script.to_data(false);
    return{ bitcoin_short_hash(data), version };
}

// Cast operators.
// ----------------------------------------------------------------------------

payment_address::operator const bool() const
{
    return valid_;
}

payment_address::operator const short_hash&() const
{
    return hash_;
}

// Serializer.
// ----------------------------------------------------------------------------

std::string payment_address::encoded() const {
    return encode_base58(wrap(version_, hash_));
}

#ifdef BITPRIM_CURRENCY_BCH

// Convert the data part to a 5 bit representation.
template <typename T>
data_chunk pack_addr_data_(T const& id, uint8_t type) {
    uint8_t version_byte(type << 3);
    size_t size = id.size();
    uint8_t encoded_size = 0;

    switch (size * 8) {
        case 160:
            encoded_size = 0;
            break;
        case 192:
            encoded_size = 1;
            break;
        case 224:
            encoded_size = 2;
            break;
        case 256:
            encoded_size = 3;
            break;
        case 320:
            encoded_size = 4;
            break;
        case 384:
            encoded_size = 5;
            break;
        case 448:
            encoded_size = 6;
            break;
        case 512:
            encoded_size = 7;
            break;
        default:
            throw std::runtime_error("Error packing cashaddr: invalid address length");
    }

    version_byte |= encoded_size;
    data_chunk data = { version_byte };
    data.insert(data.end(), std::begin(id), std::end(id));

    data_chunk converted;
    // Reserve the number of bytes required for a 5-bit packed version of a
    // hash, with version byte.  Add half a byte(4) so integer math provides
    // the next multiple-of-5 that would fit all the data.
    converted.reserve(((size + 1) * 8 + 4) / 5);
    ConvertBits<8, 5, true>(converted, std::begin(data), std::end(data));

    return converted;
}

std::string encode_cashaddr_(payment_address const& wallet) {
    if (wallet.version() == payment_address::mainnet_p2kh || wallet.version() == payment_address::mainnet_p2sh) {
        // Mainnet
        return cashaddr::encode(payment_address::cashaddr_prefix_mainnet, pack_addr_data_(wallet.hash(), wallet.version() == payment_address::mainnet_p2kh ? PUBKEY_TYPE : SCRIPT_TYPE));
    } else if (wallet.version() == payment_address::testnet_p2kh || wallet.version() == payment_address::testnet_p2sh) {
        // Testnet
        return cashaddr::encode(payment_address::cashaddr_prefix_testnet, pack_addr_data_(wallet.hash(), wallet.version() == payment_address::testnet_p2kh ? PUBKEY_TYPE : SCRIPT_TYPE));
    }
    return "";
}

std::string payment_address::encoded_cashaddr() const {
    return encode_cashaddr_(*this);
}

#endif //BITPRIM_CURRENCY_BCH

// Accessors.
// ----------------------------------------------------------------------------

uint8_t payment_address::version() const
{
    return version_;
}

const short_hash& payment_address::hash() const
{
    return hash_;
}

// Methods.
// ----------------------------------------------------------------------------

payment payment_address::to_payment() const
{
    return wrap(version_, hash_);
}

// Operators.
// ----------------------------------------------------------------------------

payment_address& payment_address::operator=(const payment_address& other)
{
    valid_ = other.valid_;
    version_ = other.version_;
    hash_ = other.hash_;
    return *this;
}

bool payment_address::operator<(const payment_address& other) const
{
    return encoded() < other.encoded();
}

bool payment_address::operator==(const payment_address& other) const
{
    return valid_ == other.valid_ && version_ == other.version_ &&
        hash_ == other.hash_;
}

bool payment_address::operator!=(const payment_address& other) const
{
    return !(*this == other);
}

std::istream& operator>>(std::istream& in, payment_address& to)
{
    std::string value;
    in >> value;
    to = payment_address(value);

    if (!to)
    {
        using namespace boost::program_options;
        BOOST_THROW_EXCEPTION(invalid_option_value(value));
    }

    return in;
}

std::ostream& operator<<(std::ostream& out, const payment_address& of)
{
    out << of.encoded();
    return out;
}

// Static functions.
// ----------------------------------------------------------------------------

// Context free input extraction is provably ambiguous (see extract_input).
payment_address::list payment_address::extract(const chain::script& script,
    uint8_t p2kh_version, uint8_t p2sh_version)
{
    const auto input = extract_input(script, p2kh_version, p2sh_version);
    return input.empty() ? extract_output(script, p2kh_version, p2sh_version) :
        input;
}

// Context free input extraction is provably ambiguous. See inline comments.
payment_address::list payment_address::extract_input(
    const chain::script& script, uint8_t p2kh_version, uint8_t p2sh_version)
{
    // A sign_key_hash result always implies sign_script_hash as well.
    const auto pattern = script.input_pattern();

    switch (pattern)
    {
        // Given lack of context (prevout) sign_key_hash is always ambiguous
        // with sign_script_hash, so return both potentially-correct addresses.
        // A server can differentiate by extracting from the previous output.
        case script_pattern::sign_key_hash:
        {
            return
            {
                { ec_public{ script[1].data() }, p2kh_version },
                { bitcoin_short_hash(script.back().data()), p2sh_version }
            };
        }
        case script_pattern::sign_script_hash:
        {
            return
            {
                { bitcoin_short_hash(script.back().data()), p2sh_version }
            };
        }

        // There is no address in sign_public_key script (signature only)
        // and the public key cannot be extracted from the signature.
        // Given lack of context (prevout) sign_public_key is always ambiguous
        // with sign_script_hash (though actual conflict seems very unlikely).
        // A server can obtain by extracting from the previous output.
        case script_pattern::sign_public_key:

        // There are no addresses in sign_multisig script, signatures only.
        // Nonstandard (non-zero) first op sign_multisig may conflict with
        // sign_key_hash and/or sign_script_hash (or will be non_standard).
        // A server can obtain the public keys extracting from the previous
        // output, but bare multisig does not associate a payment address.
        case script_pattern::sign_multisig:
        case script_pattern::non_standard:
        default:
        {
            return{};
        }
    }
}

// A server should use this against the prevout instead of using extract_input.
payment_address::list payment_address::extract_output(
    const chain::script& script, uint8_t p2kh_version, uint8_t p2sh_version)
{
    const auto pattern = script.output_pattern();

    switch (pattern)
    {
        case script_pattern::pay_key_hash:
        {
            return
            {
                { to_array<short_hash_size>(script[2].data()), p2kh_version }
            };
        }
        case script_pattern::pay_script_hash:
        {
            return
            {
                { to_array<short_hash_size>(script[1].data()), p2sh_version }
            };
        }
        case script_pattern::pay_public_key:
        {
            return
            {
                // pay_public_key is not p2kh but we conflate for tracking.
                { ec_public{ script[0].data() }, p2kh_version }
            };
        }

        // Bare multisig and null data do not associate a payment address.
        case script_pattern::pay_multisig:
        case script_pattern::null_data:
        case script_pattern::non_standard:
        default:
        {
            return{};
        }
    }
}


payment_address payment_address::extract(chainv2::script const& script, uint8_t p2kh_version, uint8_t p2sh_version) {
    if (!script.is_valid())
        return{};

    short_hash hash;
    const auto pattern = script.pattern();

    // Split out the assertions for readability.
    // We know that the script is valid and can therefore rely on these.
    switch (pattern) {
        // pay
        // --------------------------------------------------------------------
        case script_pattern::pay_multisig:
            break;
        case script_pattern::pay_public_key:
            BITCOIN_ASSERT(script.size() == 2);
            BITCOIN_ASSERT(
                script[0].data().size() == ec_compressed_size ||
                script[0].data().size() == ec_uncompressed_size);
            break;
        case script_pattern::pay_key_hash:
            BITCOIN_ASSERT(script.size() == 5);
            BITCOIN_ASSERT(script[2].data().size() == short_hash_size);
            break;
        case script_pattern::pay_script_hash:
            BITCOIN_ASSERT(script.size() == 3);
            BITCOIN_ASSERT(script[1].data().size() == short_hash_size);
            break;

        // sign
        // --------------------------------------------------------------------
        case script_pattern::sign_multisig:
            break;
        case script_pattern::sign_public_key:
            break;
        case script_pattern::sign_key_hash:
            BITCOIN_ASSERT(script.size() == 2);
            BITCOIN_ASSERT(
                script[1].data().size() == ec_compressed_size ||
                script[1].data().size() == ec_uncompressed_size);
            break;
        case script_pattern::sign_script_hash:
            BITCOIN_ASSERT(script.size() > 1);
            break;
        case script_pattern::non_standard:
        default:;
    }

    // Convert data to hash or point and construct address.
    switch (pattern) {
        // pay
        // --------------------------------------------------------------------

        // TODO: extract addresses into a vector result.
        case script_pattern::pay_multisig:
            return{};

        case script_pattern::pay_public_key: {
            const auto& data = script[0].data();
            if (data.size() == ec_compressed_size) {
                const auto point = to_array<ec_compressed_size>(data);
                return payment_address(point, p2kh_version);
            }

            const auto point = to_array<ec_uncompressed_size>(data);
            return payment_address(point, p2kh_version);
        }

        case script_pattern::pay_key_hash:
            hash = to_array<short_hash_size>(script[2].data());
            return payment_address(hash, p2kh_version);

        case script_pattern::pay_script_hash:
            hash = to_array<short_hash_size>(script[1].data());
            return payment_address(hash, p2sh_version);

        // sign
        // --------------------------------------------------------------------

        // TODO: extract addresses into a vector result.
        case script_pattern::sign_multisig:
            return{};

        // There is no address in a sign_public_key script.
        case script_pattern::sign_public_key:
            return{};

        case script_pattern::sign_key_hash: {
            const auto& data = script[1].data();
            if (data.size() == ec_compressed_size) {
                const auto point = to_array<ec_compressed_size>(data);
                return payment_address(point, p2kh_version);
            }

            const auto point = to_array<ec_uncompressed_size>(data);
            return payment_address(point, p2kh_version);
        }

        case script_pattern::sign_script_hash:
            hash = bitcoin_short_hash(script.back().data());
            return payment_address(hash, p2sh_version);

        case script_pattern::non_standard:
        default:
            return{};
    }
}



} // namespace wallet
} // namespace libbitcoin