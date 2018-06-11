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
#include <bitcoin/bitcoin/chain/chain_state.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>

#include <boost/range/adaptor/reversed.hpp>
#include <bitcoin/bitcoin/chain/block.hpp>
#include <bitcoin/bitcoin/chain/chain_state.hpp>
#include <bitcoin/bitcoin/chain/compact.hpp>
#include <bitcoin/bitcoin/chain/script.hpp>
#include <bitcoin/bitcoin/config/checkpoint.hpp>
#include <bitcoin/bitcoin/constants.hpp>
#include <bitcoin/bitcoin/math/hash.hpp>
#include <bitcoin/bitcoin/math/limits.hpp>
#include <bitcoin/bitcoin/machine/opcode.hpp>
#include <bitcoin/bitcoin/machine/rule_fork.hpp>
#include <bitcoin/bitcoin/multi_crypto_support.hpp>
#include <bitcoin/bitcoin/unicode/unicode.hpp>
#include <bitcoin/bitcoin/utility/timer.hpp>



namespace libbitcoin {
namespace chain {

using namespace bc::config;
using namespace bc::machine;
using namespace boost::adaptors;

// Inlines.
//-----------------------------------------------------------------------------

inline size_t version_sample_size(bool mainnet)
{
    return mainnet ? mainnet_sample : testnet_sample;
}

inline bool is_active(size_t count, bool mainnet)
{
    return count >= (mainnet ? mainnet_active : testnet_active);
}

inline bool is_enforced(size_t count, bool mainnet)
{
    return count >= (mainnet ? mainnet_enforce : testnet_enforce);
}

inline bool is_bip16_exception(const checkpoint& check, bool mainnet)
{
    return mainnet && check == mainnet_bip16_exception_checkpoint;
}

inline bool is_bip30_exception(const checkpoint& check, bool mainnet)
{
    return mainnet &&
        ((check == mainnet_bip30_exception_checkpoint1) ||
         (check == mainnet_bip30_exception_checkpoint2));
}

inline bool allow_collisions(const hash_digest& hash, bool mainnet,
    bool testnet)
{
    const auto regtest = !mainnet && !testnet;
    return
        (mainnet && hash == mainnet_bip34_active_checkpoint.hash()) ||
        (testnet && hash == testnet_bip34_active_checkpoint.hash()) ||
        (regtest && hash == regtest_bip34_active_checkpoint.hash());
}

inline bool allow_collisions(size_t height, bool mainnet, bool testnet)
{
    const auto regtest = !mainnet && !testnet;
    return
        (mainnet && height == mainnet_bip34_active_checkpoint.height()) ||
        (testnet && height == testnet_bip34_active_checkpoint.height()) ||
        (regtest && height == regtest_bip34_active_checkpoint.height());
}

inline bool bip9_bit0_active(const hash_digest& hash, bool mainnet,
    bool testnet)
{
    const auto regtest = !mainnet && !testnet;
    return
        (mainnet && hash == mainnet_bip9_bit0_active_checkpoint.hash()) ||
        (testnet && hash == testnet_bip9_bit0_active_checkpoint.hash()) ||
        (regtest && hash == regtest_bip9_bit0_active_checkpoint.hash());
}

inline bool bip9_bit0_active(size_t height, bool mainnet, bool testnet)
{
    const auto regtest = !mainnet && !testnet;
    return
        (mainnet && height == mainnet_bip9_bit0_active_checkpoint.height()) ||
        (testnet && height == testnet_bip9_bit0_active_checkpoint.height()) ||
        (regtest && height == regtest_bip9_bit0_active_checkpoint.height());
}

inline bool bip9_bit1_active(const hash_digest& hash, bool mainnet,
    bool testnet)
{
#ifdef BITPRIM_CURRENCY_BCH
    return false;
#endif
    const auto regtest = !mainnet && !testnet;
    return
        (mainnet && hash == mainnet_bip9_bit1_active_checkpoint.hash()) ||
        (testnet && hash == testnet_bip9_bit1_active_checkpoint.hash()) ||
        (regtest && hash == regtest_bip9_bit1_active_checkpoint.hash());
}

inline bool bip9_bit1_active(size_t height, bool mainnet, bool testnet)
{
#ifdef BITPRIM_CURRENCY_BCH
    return false;
#endif
    const auto regtest = !mainnet && !testnet;
    return
        (mainnet && height == mainnet_bip9_bit1_active_checkpoint.height()) ||
        (testnet && height == testnet_bip9_bit1_active_checkpoint.height()) ||
        (regtest && height == regtest_bip9_bit1_active_checkpoint.height());
}

inline bool bip34(size_t height, bool frozen, bool mainnet, bool testnet)
{
    const auto regtest = !mainnet && !testnet;
    return frozen &&
        ((mainnet && height >= mainnet_bip34_freeze) ||
         (testnet && height >= testnet_bip34_freeze) ||
         (regtest && height >= regtest_bip34_freeze));
}

inline bool bip66(size_t height, bool frozen, bool mainnet, bool testnet)
{
    const auto regtest = !mainnet && !testnet;
    return frozen &&
        ((mainnet && height >= mainnet_bip66_freeze) ||
         (testnet && height >= testnet_bip66_freeze) ||
         (regtest && height >= regtest_bip66_freeze));
}

inline bool bip65(size_t height, bool frozen, bool mainnet, bool testnet)
{
    const auto regtest = !mainnet && !testnet;
    return frozen &&
        ((mainnet && height >= mainnet_bip65_freeze) ||
         (testnet && height >= testnet_bip65_freeze) ||
         (regtest && height >= regtest_bip65_freeze));
}

inline uint32_t timestamp_high(chain_state::data const& values)
{
    return values.timestamp.ordered.back();
}

inline uint32_t bits_high(chain_state::data const& values)
{
    return values.bits.ordered.back();
}

#ifdef BITPRIM_CURRENCY_BCH
uint256_t chain_state::difficulty_adjustment_cash(uint256_t target) {
    return target + (target >> 2);
}

// inline constexpr
// bool is_bch_daa_enabled(uint32_t median_time_past) {
//     return (median_time_past >= bch_daa_activation_time);
// }

//static
inline
bool chain_state::is_mtp_activated(uint32_t median_time_past, uint32_t activation_time) {
    return (median_time_past >= activation_time);
}

bool chain_state::is_monolith_enabled() const {
    return is_mtp_activated(median_time_past(), monolith_activation_time());
}

bool chain_state::is_replay_protection_enabled() const {
    return is_mtp_activated(median_time_past(), magnetic_anomaly_activation_time());
}
#endif //BITPRIM_CURRENCY_BCH


// Statics.
// activation
//-----------------------------------------------------------------------------

chain_state::activations chain_state::activation(data const& values, uint32_t forks
#ifdef BITPRIM_CURRENCY_BCH
                                                , uint64_t monolith_activation_time, uint64_t magnetic_anomaly_activation_time
#endif //BITPRIM_CURRENCY_BCH
) {
    auto const height = values.height;
    auto const version = values.version.self;
    auto const& history = values.version.ordered;
    auto const frozen = script::is_enabled(forks, rule_fork::bip90_rule);
    auto const testnet = script::is_enabled(forks, rule_fork::easy_blocks);
    const auto retarget = script::is_enabled(forks, rule_fork::retarget);
    const auto mainnet = retarget && !testnet;

    //*************************************************************************
    // CONSENSUS: Though unspecified in bip34, the satoshi implementation
    // performed this comparison using the signed integer version value.
    //*************************************************************************
    auto const ge = [](uint32_t value, size_t version) {
        return static_cast<int32_t>(value) >= version;
    };

    // Declare bip34-based version predicates.
    auto const ge_2 = [=](uint32_t value) { return ge(value, bip34_version); };
    auto const ge_3 = [=](uint32_t value) { return ge(value, bip66_version); };
    auto const ge_4 = [=](uint32_t value) { return ge(value, bip65_version); };

    // Compute bip34-based activation version summaries.
    auto const count_2 = std::count_if(history.begin(), history.end(), ge_2);
    auto const count_3 = std::count_if(history.begin(), history.end(), ge_3);
    auto const count_4 = std::count_if(history.begin(), history.end(), ge_4);

    // Frozen activations (require version and enforce above freeze height).
    const auto bip34_ice = bip34(height, frozen, mainnet, testnet);
    const auto bip66_ice = bip66(height, frozen, mainnet, testnet);
    const auto bip65_ice = bip65(height, frozen, mainnet, testnet);

    // Initialize activation results with genesis values.
    activations result{ rule_fork::no_rules, first_version };

    // retarget is only activated via configuration (hard fork).
    result.forks |= (rule_fork::retarget & forks);

    // testnet is activated based on configuration alone (hard fork).
    result.forks |= (rule_fork::easy_blocks & forks);

    // bip90 is activated based on configuration alone (hard fork).
    result.forks |= (rule_fork::bip90_rule & forks);

    // bip16 is activated with a one-time test on mainnet/testnet (~55% rule).
    // There was one invalid p2sh tx mined after that time (code shipped late).
    if (values.timestamp.self >= bip16_activation_time &&
        !is_bip16_exception({ values.hash, height }, mainnet))
    {
        result.forks |= (rule_fork::bip16_rule & forks);
    }

    // bip30 is active for all but two mainnet blocks that violate the rule.
    // These two blocks each have a coinbase transaction that exctly duplicates
    // another that is not spent by the arrival of the corresponding duplicate.
    if (!is_bip30_exception({ values.hash, height }, mainnet))
    {
        result.forks |= (rule_fork::bip30_rule & forks);
    }


#ifdef BITPRIM_CURRENCY_LTC
    if (bip34_ice) {
        result.forks |= (rule_fork::bip34_rule & forks);
    }
#else
    // bip34 is activated based on 75% of preceding 1000 mainnet blocks.
    if (bip34_ice || (is_active(count_2, mainnet) && version >= bip34_version))
    {
        result.forks |= (rule_fork::bip34_rule & forks);
    }
#endif

    // bip66 is activated based on 75% of preceding 1000 mainnet blocks.
    if (bip66_ice || (is_active(count_3, mainnet) && version >= bip66_version))
    {
        result.forks |= (rule_fork::bip66_rule & forks);
    }

    // bip65 is activated based on 75% of preceding 1000 mainnet blocks.
    if (bip65_ice || (is_active(count_4, mainnet) && version >= bip65_version))
    {
        result.forks |= (rule_fork::bip65_rule & forks);
    }

    // allow_collisions is active above the bip34 checkpoint only.
    if (allow_collisions(values.allow_collisions_hash, mainnet, testnet))
    {
        result.forks |= (rule_fork::allow_collisions & forks);
    }

    // bip9_bit0 forks are enforced above the bip9_bit0 checkpoint.
    if (bip9_bit0_active(values.bip9_bit0_hash, mainnet, testnet))
    {
        result.forks |= (rule_fork::bip9_bit0_group & forks);
    }
#ifndef BITPRIM_CURRENCY_BCH
    // Activate the segwit rules only if not bch
    // bip9_bit1 forks are enforced above the bip9_bit1 checkpoint.
    if (bip9_bit1_active(values.bip9_bit1_hash, mainnet, testnet))
    {
        result.forks |= (rule_fork::bip9_bit1_group & forks);
    }
#endif

    // version 4/3/2 enforced based on 95% of preceding 1000 mainnet blocks.
    if (bip65_ice || is_enforced(count_4, mainnet))
    {
        result.minimum_version = bip65_version;
    }
    else if (bip66_ice || is_enforced(count_3, mainnet))
    {
        result.minimum_version = bip66_version;
    }
    else if (bip34_ice || is_enforced(count_2, mainnet))
    {
        result.minimum_version = bip34_version;
    } else {
        result.minimum_version = first_version;
    }

#ifdef BITPRIM_CURRENCY_BCH
    // if (is_bch_daa_enabled(median_time_past(values, 0))) {
    //     result.forks |= (rule_fork::cash_low_s_rule & forks);
    // }

    if (is_uahf_enabled(values.height, forks)) {
        result.forks |= (rule_fork::cash_verify_flags_script_enable_sighash_forkid & forks);
    }

    if (is_daa_enabled(values.height, forks)) {
        result.forks |= (rule_fork::cash_low_s_rule & forks);
    }

    auto mtp = median_time_past(values, 0);
    if (is_mtp_activated(mtp, monolith_activation_time)) {
        result.forks |= (rule_fork::cash_monolith_opcodes & forks);
    }

    if (is_mtp_activated(mtp, magnetic_anomaly_activation_time)) {
        result.forks |= (rule_fork::cash_replay_protection & forks);
    }

#endif //BITPRIM_CURRENCY_BCH

    return result;
}

size_t chain_state::bits_count(size_t height, uint32_t forks) {
    auto const testnet = script::is_enabled(forks, rule_fork::easy_blocks);
    const auto retarget = script::is_enabled(forks, rule_fork::retarget);
    const auto easy_work = testnet && retarget && !is_retarget_height(height);

#ifdef BITPRIM_CURRENCY_BCH
    return easy_work ? std::min(height, retargeting_interval) : std::min(height, chain_state_timestamp_count);
#else
    return easy_work ? std::min(height, retargeting_interval) : 1;
#endif //BITPRIM_CURRENCY_BCH
}

size_t chain_state::version_count(size_t height, uint32_t forks) {
    if (script::is_enabled(forks, rule_fork::bip90_rule) || ! script::is_enabled(forks, rule_fork::bip34_activations)) {
        return 0;
    }

    // Regtest and testnet both use bip34 test activation.
    const auto testnet = script::is_enabled(forks, rule_fork::easy_blocks);
    const auto retarget = script::is_enabled(forks, rule_fork::retarget);
    return std::min(height, version_sample_size(retarget && !testnet));
}

size_t chain_state::timestamp_count(size_t height, uint32_t) {

#ifdef BITPRIM_CURRENCY_BCH
    return std::min(height, chain_state_timestamp_count); //TODO(bitprim): check what happens with Bitcoin Legacy...?
#else
    return std::min(height, median_time_past_interval);
#endif //BITPRIM_CURRENCY_BCH
}

size_t chain_state::retarget_height(size_t height, uint32_t forks)
{
    if (!script::is_enabled(forks, rule_fork::retarget))
        return map::unrequested;

    // Height must be a positive multiple of interval, so underflow safe.
    // If not retarget height get most recent so that it may be promoted.
    return height - (is_retarget_height(height) ? retargeting_interval :
        retarget_distance(height));
}

size_t chain_state::collision_height(size_t height, uint32_t forks)
{
    const auto testnet = script::is_enabled(forks, rule_fork::easy_blocks);
    const auto regtest = !script::is_enabled(forks, rule_fork::retarget);

    const auto bip34_height =
        testnet ? testnet_bip34_active_checkpoint.height() :
        regtest ? regtest_bip34_active_checkpoint.height() :
            mainnet_bip34_active_checkpoint.height();

    // Require collision hash at heights above historical bip34 activation.
    return height > bip34_height ? bip34_height : map::unrequested;
}

size_t chain_state::bip9_bit0_height(size_t height, uint32_t forks)
{
    const auto testnet = script::is_enabled(forks, rule_fork::easy_blocks);
    const auto regtest = !script::is_enabled(forks, rule_fork::retarget);

    const auto activation_height =
        testnet ? testnet_bip9_bit0_active_checkpoint.height() :
        regtest ? regtest_bip9_bit0_active_checkpoint.height() :
            mainnet_bip9_bit0_active_checkpoint.height();

    // Require bip9_bit0 hash at heights above historical bip9_bit0 activation.
    return height > activation_height ? activation_height : map::unrequested;
}

size_t chain_state::bip9_bit1_height(size_t height, uint32_t forks)
{
#ifdef BITPRIM_CURRENCY_BCH
    return map::unrequested;
#endif
    const auto testnet = script::is_enabled(forks, rule_fork::easy_blocks);

    const auto activation_height = testnet ?
        testnet_bip9_bit1_active_checkpoint.height() :
        mainnet_bip9_bit1_active_checkpoint.height();

    // Require bip9_bit1 hash at heights above historical bip9_bit1 activation.
    return height > activation_height ? activation_height : map::unrequested;
}

// median_time_past
//-----------------------------------------------------------------------------

#ifdef BITPRIM_CURRENCY_BCH

inline 
size_t chain_state::uahf_height(size_t height, uint32_t forks) {
    auto const testnet = script::is_enabled(forks, rule_fork::easy_blocks);

    auto const activation_height = testnet ? testnet_uahf_active_checkpoint.height() : mainnet_uahf_active_checkpoint.height();

    return height >= activation_height ? activation_height : map::unrequested;
}

inline 
size_t chain_state::daa_height(size_t height, uint32_t forks) {
    auto const testnet = script::is_enabled(forks, rule_fork::easy_blocks);

    auto const activation_height = testnet ? testnet_daa_active_checkpoint.height() : mainnet_daa_active_checkpoint.height();

    //TODO(fernando): > or >= ??
    return height > activation_height ? activation_height : map::unrequested;
}

inline 
bool chain_state::is_uahf_enabled(size_t height, uint32_t forks) {
    auto const testnet = script::is_enabled(forks, rule_fork::easy_blocks);

    auto const activation_height = testnet ? testnet_uahf_active_checkpoint.height() : mainnet_uahf_active_checkpoint.height();

    return height >= activation_height;
}

inline 
bool chain_state::is_daa_enabled(size_t height, uint32_t forks) {
    auto const testnet = script::is_enabled(forks, rule_fork::easy_blocks);

    auto const activation_height = testnet ? testnet_daa_active_checkpoint.height() : mainnet_daa_active_checkpoint.height();

    //TODO(fernando): > or >= ??
    return height > activation_height;
}

#endif


typename chain_state::timestamps::const_iterator 
timestamps_position(chain_state::timestamps const& times, bool tip) {

#ifdef BITPRIM_CURRENCY_BCH
    if (tip) {
        if (times.size() >= bitcoin_cash_offset_tip) {
            return times.begin() + bitcoin_cash_offset_tip;
        }
    } else {
        if (times.size() >= bitcoin_cash_offset_tip_minus_6) {
            return times.begin() + bitcoin_cash_offset_tip_minus_6;
        }
    }
#endif //BITPRIM_CURRENCY_BCH


    if (times.size() > 11) {
        return times.begin() + (times.size() - 11);
    }

    return times.begin();
}

std::vector<typename chain_state::timestamps::value_type> 
timestamps_subset(chain_state::timestamps const& times, bool tip) {
    auto at = timestamps_position(times, tip);
    auto n = (std::min)((size_t)std::distance(at, times.end()), median_time_past_interval);

    std::vector<typename chain_state::timestamps::value_type> subset(n);
    std::copy_n(at, n, subset.begin());

    return subset;
}

uint32_t chain_state::median_time_past(data const& values, uint32_t, bool tip /*= true*/) {
    // Create a copy for the in-place sort.
    auto subset = timestamps_subset(values.timestamp.ordered, tip);

    // Sort the times by value to obtain the median.
    std::sort(subset.begin(), subset.end());

    // Consensus defines median time using modulo 2 element selection.
    // This differs from arithmetic median which averages two middle values.
    return subset.empty() ? 0 : subset[subset.size() / 2];
}

// ------------------------------------------------------------------------------------------------------------
// Note(fernando): This is a Non-Stable Non-Standard Median of Three Algorithm
// See Footnote 2 of https://github.com/Bitcoin-UAHF/spec/blob/master/nov-13-hardfork-spec.md
// Modified version of select_1_3 taken from https://github.com/tao-cpp/algorithm
// ------------------------------------------------------------------------------------------------------------
#define FN(body) -> decltype(body) {return body;}

template <typename T, typename U, typename R>
// requires(SameType<T, U> && Domain<R, T>)
inline constexpr
auto select_0_2(T&& a, U&& b, R r) FN(
    r(b, a) ? std::forward<U>(b) : std::forward<T>(a)
)

template <typename T, typename U, typename V, typename R>
// requires(SameType<T, U> && SameType<U, V> && Domain<R, T>)
inline constexpr
auto select_1_3_ac(T&& a, U&& b, V&& c, R r) FN(
    // precondition: ! r(c, a)
    r(b, a) ?
          std::forward<T>(a)                                      // b, a, c 
        : select_0_2(std::forward<U>(b), std::forward<V>(c), r)   // a is not the median
)

template <typename T, typename U, typename V, typename R>
// requires(SameType<T, U> && SameType<U, V> && Domain<R, T>)
inline constexpr
auto select_1_3(T&& a, U&& b, V&& c, R r) FN(
    r(c, a) ?
          select_1_3_ac(std::forward<V>(c), std::forward<U>(b), std::forward<T>(a), r) 
        : select_1_3_ac(std::forward<T>(a), std::forward<U>(b), std::forward<V>(c), r)
)

#undef FN
// ------------------------------------------------------------------------------------------------------------


#ifdef BITPRIM_CURRENCY_BCH

// DAA, New algorithm: 2017-Nov-13 Hard fork
uint32_t chain_state::cash_difficulty_adjustment(data const& values) {
    // precondition: values.timestamp.size() >= 147

    using std::make_pair;
    using pair_t = std::pair<size_t, uint32_t>;
    
    auto const bits_size = values.bits.ordered.size();

    auto pair_cmp = [](pair_t const& a, pair_t const& b) { return a.second < b.second;};

    auto first_block = select_1_3(make_pair(bits_size - 3, values.timestamp.ordered[144]), 
                                  make_pair(bits_size - 2, values.timestamp.ordered[145]),
                                  make_pair(bits_size - 1, values.timestamp.ordered[146]),
                                  pair_cmp);
    
    auto last_block = select_1_3(make_pair(bits_size - 147, values.timestamp.ordered[0]), 
                                 make_pair(bits_size - 146, values.timestamp.ordered[1]),
                                 make_pair(bits_size - 145, values.timestamp.ordered[2]),
                                 pair_cmp);

    uint256_t work = 0;
    for (size_t i = last_block.first + 1; i <= first_block.first; ++i) {
        work += header::proof(values.bits.ordered[i]);
    }

    work *= target_spacing_seconds; //10 * 60

    int64_t nActualTimespan = first_block.second - last_block.second;
    if (nActualTimespan > 288 * target_spacing_seconds) {
        nActualTimespan = 288 * target_spacing_seconds;
    } else if (nActualTimespan < 72 * target_spacing_seconds) {
        nActualTimespan = 72 * target_spacing_seconds;
    }

    work /= nActualTimespan;
    auto nextTarget = (-1 * work) / work; //Compute target result
    uint256_t pow_limit(compact{retarget_proof_of_work_limit});

    if (nextTarget.compare(pow_limit) == 1) {
        return retarget_proof_of_work_limit;
    }

    return compact(nextTarget).normal();
}

#endif //BITPRIM_CURRENCY_BCH

// work_required
//-----------------------------------------------------------------------------

uint32_t chain_state::work_required(data const& values, uint32_t forks) {
    // Invalid parameter via public interface, test is_valid for results.
    if (values.height == 0)
        return{};

    // Regtest bypasses all retargeting.
    if (!script::is_enabled(forks, rule_fork::retarget))
        return bits_high(values);

    auto last_time_span = median_time_past(values, 0, true);

#ifdef BITPRIM_CURRENCY_BCH
    // bool const daa_active = is_bch_daa_enabled(last_time_span);
    bool const daa_active = is_daa_enabled(values.height, forks);
#else
    bool const daa_active = false;
#endif //BITPRIM_CURRENCY_BCH

    if (is_retarget_height(values.height) && !(daa_active)) {
        return work_required_retarget(values);
    }

    if (script::is_enabled(forks, rule_fork::easy_blocks)) {
        return easy_work_required(values, daa_active);
    }

    

// #ifdef BITPRIM_CURRENCY_BCH
//     if (values.height >= bch_activation_height) {
//         if ( ! daa_active) {
//             auto six_time_span = median_time_past(values, 0, false);
//             // precondition: last_time_span >= six_time_span
//             if ((last_time_span - six_time_span) > (12 * 3600)) {
//                 return work_required_adjust_cash(values);
//             }
//         } else {
//             return cash_difficulty_adjustment(values);
// 	    }
//     }
// #endif //BITPRIM_CURRENCY_BCH

#ifdef BITPRIM_CURRENCY_BCH
    if (is_uahf_enabled(values.height, forks)) {
        if (is_daa_enabled(values.height, forks)) {
            return cash_difficulty_adjustment(values);
        } else {
            auto six_time_span = median_time_past(values, 0, false);
            // precondition: last_time_span >= six_time_span
            //TODO(fernando): resolve hardcoded values
            if ((last_time_span - six_time_span) > (12 * 3600)) {
                return work_required_adjust_cash(values);
            }
	    }
    }
#endif //BITPRIM_CURRENCY_BCH


    return bits_high(values);
}


#ifdef BITPRIM_CURRENCY_BCH
uint32_t chain_state::work_required_adjust_cash(data const& values) {
    const compact bits(bits_high(values));
    uint256_t target(bits);
    target = difficulty_adjustment_cash(target); //target += (target >> 2);
    static const uint256_t pow_limit(compact{ retarget_proof_of_work_limit });
    return target > pow_limit ? retarget_proof_of_work_limit : compact(target).normal();
}
#endif //BITPRIM_CURRENCY_BCH


// [CalculateNextWorkRequired]
uint32_t chain_state::work_required_retarget(data const& values) {
    const compact bits(bits_high(values));
    
#ifdef BITPRIM_CURRENCY_LTC
    uint256_t target(bits);
    static const uint256_t pow_limit("0x00000fffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
    // hash_number retarget_new;
    // retarget_new.set_compact(bits_high(values));

    bool shift = target > ((uint256_t(1) << 236) - 1);

    if (shift) {
        target >>= 1;
    }

    const int64_t high = timestamp_high(values);
    const int64_t retarget = values.timestamp.retarget;
    const int64_t actual_timespan = range_constrain(high - retarget, (int64_t)min_timespan, (int64_t)max_timespan);
    // std::cout << "high:            " << high << "\n";


    target *= actual_timespan;
    target /= target_timespan_seconds;

    if (shift) {
        target <<= 1;
    }

    // return target > pow_limit ? pow_limit.compact() : target.compact();

    return target > pow_limit ? retarget_proof_of_work_limit : compact(target).normal();


#else //BITPRIM_CURRENCY_LTC
    static const uint256_t pow_limit(compact{ retarget_proof_of_work_limit });
    BITCOIN_ASSERT_MSG(!bits.is_overflowed(), "previous block has bad bits");

    uint256_t target(bits);
    target *= retarget_timespan(values);
    target /= target_timespan_seconds;

    // The proof_of_work_limit constant is pre-normalized.
    return target > pow_limit ? retarget_proof_of_work_limit : compact(target).normal();
#endif //BITPRIM_CURRENCY_LTC
}

// Get the bounded total time spanning the highest 2016 blocks.
uint32_t chain_state::retarget_timespan(data const& values) {
    auto const high = timestamp_high(values);
    auto const retarget = values.timestamp.retarget;

    //*************************************************************************
    // CONSENSUS: subtract unsigned 32 bit numbers in signed 64 bit space in
    // order to prevent underflow before applying the range constraint.
    //*************************************************************************
    auto const timespan = cast_subtract<int64_t>(high, retarget);
    return range_constrain(timespan, min_timespan, max_timespan);
}

uint32_t chain_state::easy_work_required(data const& values, bool daa_active) {
    BITCOIN_ASSERT(values.height != 0);

    // If the time limit has passed allow a minimum difficulty block.
    if (values.timestamp.self > easy_time_limit(values))
        return retarget_proof_of_work_limit;

    auto height = values.height;
    auto& bits = values.bits.ordered;

    if (daa_active) { 
#ifdef BITPRIM_CURRENCY_BCH
        return cash_difficulty_adjustment(values);
#else
//Note: Could not happend: DAA and not BCH
#endif //BITPRIM_CURRENCY_BCH
    } else {
    // Reverse iterate the ordered-by-height list of header bits.
        for (auto bit = bits.rbegin(); bit != bits.rend(); ++bit) {
            if (is_retarget_or_non_limit(--height, *bit)) {
                return *bit;
            }
        }
    }

    // Since the set of heights is either a full retarget range or ends at
    // zero this is not reachable unless the data set is invalid.
    BITCOIN_ASSERT(false);
    return retarget_proof_of_work_limit;
}

uint32_t chain_state::easy_time_limit(chain_state::data const& values) {
    const int64_t high = timestamp_high(values);
    const int64_t spacing = easy_spacing_seconds;

    //*************************************************************************
    // CONSENSUS: add unsigned 32 bit numbers in signed 64 bit space in
    // order to prevent overflow before applying the domain constraint.
    //*************************************************************************
    return domain_constrain<uint32_t>(cast_add<int64_t>(high, spacing));
}

// A retarget height, or a block that does not have proof_of_work_limit bits.
bool chain_state::is_retarget_or_non_limit(size_t height, uint32_t bits) {
    // Zero is a retarget height, ensuring termination before height underflow.
    // This is guaranteed, just asserting here to document the safeguard.
    BITCOIN_ASSERT_MSG(is_retarget_height(0), "loop overflow potential");

    return bits != retarget_proof_of_work_limit || is_retarget_height(height);
}

// Determine if height is a multiple of retargeting_interval.
bool chain_state::is_retarget_height(size_t height) {
    return retarget_distance(height) == 0;
}

// Determine the number of blocks back to the closest retarget height.
size_t chain_state::retarget_distance(size_t height) {
    return height % retargeting_interval;
}

// Publics.
//-----------------------------------------------------------------------------

// static
chain_state::map chain_state::get_map(size_t height, checkpoints const& checkpoints, uint32_t forks) {
    if (height == 0)
        return{};

    map map;

    // The height bound of the reverse (high to low) retarget search.
    map.bits_self = height;
    map.bits.high = height - 1;
    map.bits.count = bits_count(height, forks);

    // The height bound of the median time past function.
    map.timestamp_self = height;
    map.timestamp.high = height - 1;
    map.timestamp.count = timestamp_count(height, forks);

    // The height bound of the version sample for activations.
    map.version_self = height;
    map.version.high = height - 1;
    map.version.count = version_count(height, forks);

    // The most recent past retarget height.
    map.timestamp_retarget = retarget_height(height, forks);

    // The checkpoint above which tx hash collisions are allowed to occur.
    map.allow_collisions_height = collision_height(height, forks);

    // The checkpoint above which bip9_bit0 rules are enforced.
    map.bip9_bit0_height = bip9_bit0_height(height, forks);

    // The checkpoint above which bip9_bit1 rules are enforced.
    map.bip9_bit1_height = bip9_bit1_height(height, forks);

    return map;
}

// static
uint32_t chain_state::signal_version(uint32_t forks) {
    if (script::is_enabled(forks, rule_fork::bip65_rule))
        return bip65_version;

    if (script::is_enabled(forks, rule_fork::bip66_rule))
        return bip66_version;

    if (script::is_enabled(forks, rule_fork::bip34_rule))
        return bip34_version;

    // TODO: these can be retired.
    // Signal bip9 bit0 if any of the group is configured.
    if (script::is_enabled(forks, rule_fork::bip9_bit0_group))
        return bip9_version_base | bip9_version_bit0;

    // TODO: these can be retired.
    // Signal bip9 bit1 if any of the group is configured.
    if (script::is_enabled(forks, rule_fork::bip9_bit1_group))
        return bip9_version_base | bip9_version_bit1;

    return first_version;
}

// This is promotion from a preceding height to the next.
chain_state::data chain_state::to_pool(const chain_state& top)
{
    // Alias configured forks.
    const auto forks = top.forks_;

    // Retargeting is only activated via configuration.
    const auto retarget = script::is_enabled(forks, rule_fork::retarget);

    // Copy data from presumed previous-height block state.
    auto data = top.data_;

    // If this overflows height is zero and result is handled as invalid.
    auto const height = data.height + 1u;

    // Enqueue previous block values to collections.
    data.bits.ordered.push_back(data.bits.self);
    data.version.ordered.push_back(data.version.self);
    data.timestamp.ordered.push_back(data.timestamp.self);

    // If bits collection overflows, dequeue oldest member.
    if (data.bits.ordered.size() > bits_count(height, forks))
        data.bits.ordered.pop_front();

    // If version collection overflows, dequeue oldest member.
    if (data.version.ordered.size() > version_count(height, forks))
        data.version.ordered.pop_front();

    // If timestamp collection overflows, dequeue oldest member.
    if (data.timestamp.ordered.size() > timestamp_count(height, forks))
        data.timestamp.ordered.pop_front();

    // Regtest does not perform retargeting.
    // If promoting from retarget height, move that timestamp into retarget.
    if (retarget && is_retarget_height(height - 1u))
        data.timestamp.retarget = data.timestamp.self;

    // Replace previous block state with tx pool chain state for next height.
    // Only height and version used by tx pool, others promotable or unused.
    // Preserve data.allow_collisions_hash promotion.
    // Preserve data.bip9_bit0_hash promotion.
    // Preserve data.bip9_bit1_hash promotion.
    data.height = height;
    data.hash = null_hash;
    data.bits.self = work_limit(retarget);
    data.version.self = signal_version(forks);
    data.timestamp.self = max_uint32;
    return data;
}

// Constructor (top to pool).
// This generates a state for the pool above the presumed top block state.
chain_state::chain_state(chain_state const& top)
    : data_(to_pool(top))
    , forks_(top.forks_)
    , checkpoints_(top.checkpoints_)
    , active_(activation(data_, forks_
#ifdef BITPRIM_CURRENCY_BCH
        , top.monolith_activation_time_ , top.magnetic_anomaly_activation_time_
#endif //BITPRIM_CURRENCY_BCH
    ))
    , median_time_past_(median_time_past(data_, forks_))
    , work_required_(work_required(data_, forks_))
#ifdef BITPRIM_CURRENCY_BCH
    , monolith_activation_time_(top.monolith_activation_time_)
    , magnetic_anomaly_activation_time_(top.magnetic_anomaly_activation_time_)
#endif //BITPRIM_CURRENCY_BCH
{}

chain_state::data chain_state::to_block(const chain_state& pool,
    const block& block)
{
    // Alias configured forks.
    const auto forks = pool.forks_;

    // Retargeting and testnet are only activated via configuration.
    const auto testnet = script::is_enabled(forks, rule_fork::easy_blocks);
    const auto retarget = script::is_enabled(forks, rule_fork::retarget);
    const auto mainnet = retarget && !testnet;

    // Copy data from presumed same-height pool state.
    auto data = pool.data_;

    // Replace pool chain state with block state at same (next) height.
    // Preserve data.timestamp.retarget promotion.
    auto const& header = block.header();
    data.hash = header.hash();
    data.bits.self = header.bits();
    data.version.self = header.version();
    data.timestamp.self = header.timestamp();

    // Cache hash of bip34 height block, otherwise use preceding state.
    if (allow_collisions(data.height, mainnet, testnet))
        data.allow_collisions_hash = data.hash;

    // Cache hash of bip9 bit0 height block, otherwise use preceding state.
    if (bip9_bit0_active(data.height, mainnet, testnet))
        data.bip9_bit0_hash = data.hash;

#ifndef BITPRIM_CURRENCY_BCH
    // Cache hash of bip9 bit1 height block, otherwise use preceding state.
    if (bip9_bit1_active(data.height, mainnet, testnet))
        data.bip9_bit1_hash = data.hash;
#endif

    return data;
}

// Constructor (tx pool to block).
// This assumes that the pool state is the same height as the block.
chain_state::chain_state(chain_state const& pool, const block& block)
    : data_(to_block(pool, block))
    , forks_(pool.forks_)
    , checkpoints_(pool.checkpoints_)
    , active_(activation(data_, forks_
#ifdef BITPRIM_CURRENCY_BCH
        , pool.monolith_activation_time_ , pool.magnetic_anomaly_activation_time_
#endif //BITPRIM_CURRENCY_BCH
    ))
    , median_time_past_(median_time_past(data_, forks_))
    , work_required_(work_required(data_, forks_))
#ifdef BITPRIM_CURRENCY_BCH
    , monolith_activation_time_(pool.monolith_activation_time_)
    , magnetic_anomaly_activation_time_(pool.magnetic_anomaly_activation_time_)
#endif //BITPRIM_CURRENCY_BCH
{}

chain_state::data chain_state::to_header(const chain_state& parent,
    const header& header)
{
    // Alias configured forks.
    const auto forks = parent.forks_;

    // Retargeting and testnet are only activated via configuration.
    const auto testnet = script::is_enabled(forks, rule_fork::easy_blocks);
    const auto retarget = script::is_enabled(forks, rule_fork::retarget);
    const auto mainnet = retarget && !testnet;

    // Copy and promote data from presumed parent-height header/block state.
    auto data = to_pool(parent);

    // Replace the pool (empty) current block state with given header state.
    // Preserve data.timestamp.retarget promotion.
    data.hash = header.hash();
    data.bits.self = header.bits();
    data.version.self = header.version();
    data.timestamp.self = header.timestamp();

    // Cache hash of bip34 height block, otherwise use preceding state.
    if (allow_collisions(data.height, mainnet, testnet))
        data.allow_collisions_hash = data.hash;

    // Cache hash of bip9 bit0 height block, otherwise use preceding state.
    if (bip9_bit0_active(data.height, mainnet, testnet))
        data.bip9_bit0_hash = data.hash;

#ifndef BITPRIM_CURRENCY_BCH
    // Cache hash of bip9 bit1 height block, otherwise use preceding state.
    if (bip9_bit1_active(data.height, mainnet, testnet))
        data.bip9_bit1_hash = data.hash;
#endif

    return data;
}

// Constructor (parent to header).
// This assumes that parent is the state of the header's previous block.
chain_state::chain_state(chain_state const& parent, header const& header)
    : data_(to_header(parent, header))
    , forks_(parent.forks_)
    , checkpoints_(parent.checkpoints_)
    , active_(activation(data_, forks_
#ifdef BITPRIM_CURRENCY_BCH
        , parent.monolith_activation_time_ , parent.magnetic_anomaly_activation_time_
#endif //BITPRIM_CURRENCY_BCH
        ))
    , median_time_past_(median_time_past(data_, forks_))
    , work_required_(work_required(data_, forks_))
#ifdef BITPRIM_CURRENCY_BCH
    , monolith_activation_time_(parent.monolith_activation_time_)
    , magnetic_anomaly_activation_time_(parent.magnetic_anomaly_activation_time_)
#endif //BITPRIM_CURRENCY_BCH
{}

// Constructor (from raw data).
// The allow_collisions hard fork is always activated (not configurable).
chain_state::chain_state(data&& values, checkpoints const& checkpoints, uint32_t forks
#ifdef BITPRIM_CURRENCY_BCH
, uint64_t monolith_activation_time
, uint64_t magnetic_anomaly_activation_time
#endif //BITPRIM_CURRENCY_BCH
)
    : data_(std::move(values))
    , forks_(forks | rule_fork::allow_collisions)
    , checkpoints_(checkpoints)
    , active_(activation(data_, forks_
#ifdef BITPRIM_CURRENCY_BCH
        , monolith_activation_time, magnetic_anomaly_activation_time
#endif //BITPRIM_CURRENCY_BCH
    ))
    , median_time_past_(median_time_past(data_, forks_))
    , work_required_(work_required(data_, forks_))
#ifdef BITPRIM_CURRENCY_BCH
    , monolith_activation_time_(monolith_activation_time)
    , magnetic_anomaly_activation_time_(magnetic_anomaly_activation_time)
#endif //BITPRIM_CURRENCY_BCH
{}

// Semantic invalidity can also arise from too many/few values in the arrays.
// The same computations used to specify the ranges could detect such errors.
// These are the conditions that would cause exception during execution.
bool chain_state::is_valid() const {
    return data_.height != 0;
}

// Properties.
//-----------------------------------------------------------------------------

size_t chain_state::height() const {
    return data_.height;
}

uint32_t chain_state::enabled_forks() const {
    return active_.forks;
}

uint32_t chain_state::minimum_version() const {
    return active_.minimum_version;
}

uint32_t chain_state::median_time_past() const {
    return median_time_past_;
}

uint32_t chain_state::work_required() const {
    return work_required_;
}

#ifdef BITPRIM_CURRENCY_BCH
uint64_t chain_state::monolith_activation_time() const {
    return monolith_activation_time_;
}

uint64_t chain_state::magnetic_anomaly_activation_time() const {
    return magnetic_anomaly_activation_time_;
}
#endif //BITPRIM_CURRENCY_BCH

// Forks.
//-----------------------------------------------------------------------------

bool chain_state::is_enabled(rule_fork fork) const {
    return script::is_enabled(active_.forks, fork);
}

bool chain_state::is_checkpoint_conflict(const hash_digest& hash) const {
    return !checkpoint::validate(hash, data_.height, checkpoints_);
}

bool chain_state::is_under_checkpoint() const {
    // This assumes that the checkpoints are sorted.
    return checkpoint::covered(data_.height, checkpoints_);
}

// Mining.
//-----------------------------------------------------------------------------

uint32_t chain_state::get_next_work_required(uint32_t time_now) {
    auto values = this->data_;
    values.timestamp.self = time_now;
    return work_required(values, this->enabled_forks());
}

}} // namespace libbitcoin::chain
