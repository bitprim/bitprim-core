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

#include <bitcoin/bitcoin/bitcoin_cash_support.hpp>
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
#include <bitcoin/bitcoin/unicode/unicode.hpp>
#include <bitcoin/bitcoin/utility/timer.hpp>



namespace libbitcoin {
namespace chain {

using namespace bc::config;
using namespace bc::machine;

// Inlines.
//-----------------------------------------------------------------------------

inline size_t version_sample_size(bool testnet)
{
    return testnet ? testnet_sample : mainnet_sample;
}

inline bool is_active(size_t count, bool testnet)
{
    return count >= (testnet ? testnet_active : mainnet_active);
}

inline bool is_enforced(size_t count, bool testnet)
{
    return count >= (testnet ? testnet_enforce : mainnet_enforce);
}

inline bool is_bip16_exception(const checkpoint& check, bool testnet)
{
    return !testnet && check == mainnet_bip16_exception_checkpoint;
}

inline bool is_bip30_exception(const checkpoint& check, bool testnet)
{
    return !testnet &&
        ((check == mainnet_bip30_exception_checkpoint1) ||
         (check == mainnet_bip30_exception_checkpoint2));
}

inline bool allow_collisions(const hash_digest& hash, bool testnet)
{
    return
        ((testnet && hash == testnet_bip34_active_checkpoint.hash()) ||
        (!testnet && hash == mainnet_bip34_active_checkpoint.hash()));
}

inline bool allow_collisions(size_t height, bool testnet)
{
    return
        ((testnet && height == testnet_bip34_active_checkpoint.height()) ||
        (!testnet && height == mainnet_bip34_active_checkpoint.height()));
}

inline bool bip9_bit0_active(const hash_digest& hash, bool testnet)
{
    return
        ((testnet && hash == testnet_bip9_bit0_active_checkpoint.hash()) ||
        (!testnet && hash == mainnet_bip9_bit0_active_checkpoint.hash()));
}

inline bool bip9_bit0_active(size_t height, bool testnet)
{
    return
        ((testnet && height == testnet_bip9_bit0_active_checkpoint.height()) ||
        (!testnet && height == mainnet_bip9_bit0_active_checkpoint.height()));
}

inline bool bip34(size_t height, bool frozen, bool testnet)
{
    return frozen &&
        ((testnet && height >= testnet_bip34_freeze) ||
        (!testnet && height >= mainnet_bip34_freeze));
}

inline bool bip66(size_t height, bool frozen, bool testnet)
{
    return frozen &&
        ((testnet && height >= testnet_bip66_freeze) ||
        (!testnet && height >= mainnet_bip66_freeze));
}

inline bool bip65(size_t height, bool frozen, bool testnet)
{
    return frozen &&
        ((testnet && height >= testnet_bip65_freeze) ||
        (!testnet && height >= mainnet_bip65_freeze));
}

inline uint32_t timestamp_high(const chain_state::data& values)
{
    return values.timestamp.ordered.back();
}

inline uint32_t bits_high(const chain_state::data& values)
{
    return values.bits.ordered.back();
}

uint256_t chain_state::difficulty_adjustment_cash(uint256_t target) {
    return target + (target >> 2);
}

bool is_cash_hf_enabled(const uint32_t median_time_past) {
    return (median_time_past > bitcoin_cash_daa_activation_time);
}

// Statics.
// activation
//-----------------------------------------------------------------------------

chain_state::activations chain_state::activation(const data& values,
    uint32_t forks)
{
    const auto height = values.height;
    const auto version = values.version.self;
    const auto& history = values.version.ordered;
    const auto frozen = script::is_enabled(forks, rule_fork::bip90_rule);
    const auto testnet = script::is_enabled(forks, rule_fork::easy_blocks);

    //*************************************************************************
    // CONSENSUS: Though unspecified in bip34, the satoshi implementation
    // performed this comparison using the signed integer version value.
    //*************************************************************************
    const auto ge = [](uint32_t value, size_t version)
    {
        return static_cast<int32_t>(value) >= version;
    };

    // Declare bip34-based version predicates.
    const auto ge_2 = [=](uint32_t value) { return ge(value, bip34_version); };
    const auto ge_3 = [=](uint32_t value) { return ge(value, bip66_version); };
    const auto ge_4 = [=](uint32_t value) { return ge(value, bip65_version); };

    // Compute bip34-based activation version summaries.
    const auto count_2 = std::count_if(history.begin(), history.end(), ge_2);
    const auto count_3 = std::count_if(history.begin(), history.end(), ge_3);
    const auto count_4 = std::count_if(history.begin(), history.end(), ge_4);

    // Frozen activations (require version and enforce above freeze height).
    const auto bip34_ice = bip34(height, frozen, testnet);
    const auto bip66_ice = bip66(height, frozen, testnet);
    const auto bip65_ice = bip65(height, frozen, testnet);

    // Initialize activation results with genesis values.
    activations result{ rule_fork::no_rules, first_version };

    // testnet is activated based on configuration alone (hard fork).
    result.forks |= (rule_fork::easy_blocks & forks);

    // bip90 is activated based on configuration alone (hard fork).
    result.forks |= (rule_fork::bip90_rule & forks);

    // bip16 is activated with a one-time test on mainnet/testnet (~55% rule).
    // There was one invalid p2sh tx mined after that time (code shipped late).
    if (values.timestamp.self >= bip16_activation_time &&
        !is_bip16_exception({ values.hash, height }, testnet))
    {
        result.forks |= (rule_fork::bip16_rule & forks);
    }

    // bip30 is active for all but two mainnet blocks that violate the rule.
    // These two blocks each have a coinbase transaction that exctly duplicates
    // another that is not spent by the arrival of the corresponding duplicate.
    if (!is_bip30_exception({ values.hash, height }, testnet))
    {
        result.forks |= (rule_fork::bip30_rule & forks);
    }


#ifdef LITECOIN
    if (bip34_ice)
    {
        result.forks |= (rule_fork::bip34_rule & forks);
    }
#else
    // bip34 is activated based on 75% of preceding 1000 mainnet blocks.
    if (bip34_ice || (is_active(count_2, testnet) && version >= bip34_version))
    {
        result.forks |= (rule_fork::bip34_rule & forks);
    }
#endif

    // bip66 is activated based on 75% of preceding 1000 mainnet blocks.
    if (bip66_ice || (is_active(count_3, testnet) && version >= bip66_version))
    {
        result.forks |= (rule_fork::bip66_rule & forks);
    }

    // bip65 is activated based on 75% of preceding 1000 mainnet blocks.
    if (bip65_ice || (is_active(count_4, testnet) && version >= bip65_version))
    {
        result.forks |= (rule_fork::bip65_rule & forks);
    }

    // allow_collisions is active above the bip34 checkpoint only.
    if (allow_collisions(values.allow_collisions_hash, testnet))
    {
        result.forks |= (rule_fork::allow_collisions & forks);
    }

    // bip9_bit0 forks are enforced above the bip9_bit0 checkpoint.
    if (bip9_bit0_active(values.bip9_bit0_hash, testnet))
    {
        result.forks |= (rule_fork::bip9_bit0_group & forks);
    }

    // version 4/3/2 enforced based on 95% of preceding 1000 mainnet blocks.
    if (bip65_ice || is_enforced(count_4, testnet))
    {
        result.minimum_version = bip65_version;
    }
    else if (bip66_ice || is_enforced(count_3, testnet))
    {
        result.minimum_version = bip66_version;
    }
    else if (bip34_ice || is_enforced(count_2, testnet))
    {
        result.minimum_version = bip34_version;
    }
    else
    {
        result.minimum_version = first_version;
    }

    if (is_bitcoin_cash() && is_cash_hf_enabled(median_time_past(values, 0)))
    {
        result.forks |= (rule_fork::cash_low_s_rule & forks);
    }
    return result;
}

size_t chain_state::bits_count(size_t height, uint32_t forks)
{
    const auto testnet = script::is_enabled(forks, rule_fork::easy_blocks);
    const auto easy_work = testnet && !is_retarget_height(height);
    return easy_work ? std::min(height, retargeting_interval) : std::min(height, chain_state_timestamp_count);
}

size_t chain_state::version_count(size_t height, uint32_t forks)
{
    if (script::is_enabled(forks, rule_fork::bip90_rule) ||
        !script::is_enabled(forks, rule_fork::bip34_activations))
    {
        return 0;
    }

    const auto testnet = script::is_enabled(forks, rule_fork::easy_blocks);
    return std::min(height, version_sample_size(testnet));
}

size_t chain_state::timestamp_count(size_t height, uint32_t)
{
    // const auto checked = is_checkpointed(height, checkpoints);
    // return checked ? 1 : std::min(height, chain_state_timestamp_count); //TODO(bitprim): check what happens with Bitcoin Legacy...?
    return std::min(height, chain_state_timestamp_count); //TODO(bitprim): check what happens with Bitcoin Legacy...?
}

size_t chain_state::retarget_height(size_t height, uint32_t)
{
    // Height must be a positive multiple of interval, so underflow safe.
    // If not retarget height get most recent so that it may be promoted.
    return height - (is_retarget_height(height) ? retargeting_interval :
        retarget_distance(height));
}

size_t chain_state::collision_height(size_t height, uint32_t forks)
{
    const auto testnet = script::is_enabled(forks, rule_fork::easy_blocks);

    const auto bip34_height = testnet ?
        testnet_bip34_active_checkpoint.height() :
        mainnet_bip34_active_checkpoint.height();

    // Require collision hash at heights above historical bip34 activation.
    return height > bip34_height ? bip34_height : map::unrequested;
}

size_t chain_state::bip9_bit0_height(size_t height, uint32_t forks)
{
    const auto testnet = script::is_enabled(forks, rule_fork::easy_blocks);

    const auto activation_height = testnet ?
        testnet_bip9_bit0_active_checkpoint.height() :
        mainnet_bip9_bit0_active_checkpoint.height();

    // Require bip9_bit0 hash at heights above historical bip9_bit0 activation.
    return height > activation_height ? activation_height : map::unrequested;
}

typename chain_state::timestamps::const_iterator 
timestamps_position(chain_state::timestamps const& times, bool tip) {

    if (tip) {
        if (times.size() >= bitcoin_cash_offset_tip)
            return times.begin() + bitcoin_cash_offset_tip;
    } else {
        if (times.size() >= bitcoin_cash_offset_tip_minus_6)
            return times.begin() + bitcoin_cash_offset_tip_minus_6;
    }

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


// New algorithm
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
        work += block::proof(values.bits.ordered[i]);
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
    uint256_t pow_limit(compact{proof_of_work_limit});

    if (nextTarget.compare(pow_limit) == 1) {
        return proof_of_work_limit;
    }

    return compact(nextTarget).normal();
}

// work_required
//-----------------------------------------------------------------------------

uint32_t chain_state::work_required(const data& values, uint32_t forks) {
    // Invalid parameter via public interface, test is_valid for results.
    if (values.height == 0) {
        return{};
    }
        
    bool daa_active = false;
    auto last_time_span = median_time_past(values, 0, true);
    if ((last_time_span >= bitcoin_cash_daa_activation_time) && (is_bitcoin_cash())){
        daa_active = true;
    }

    if (is_retarget_height(values.height) && !(daa_active)) {
        return work_required_retarget(values);
    }

    if (script::is_enabled(forks, rule_fork::easy_blocks)) {
        return easy_work_required(values, daa_active);
    }

    if (is_bitcoin_cash() && values.height > bitcoin_cash_activation_height) {
        if (!daa_active) {
            auto six_time_span = median_time_past(values, 0, false);
            // precondition: last_time_span >= six_time_span
            if ((last_time_span - six_time_span) > (12 * 3600)) {
                return work_required_adjust_cash(values);
            }
        } else {
            return cash_difficulty_adjustment(values);
	}
    }

    return bits_high(values);
}

uint32_t chain_state::work_required_adjust_cash(const data& values)
{
    const compact bits(bits_high(values));
    uint256_t target(bits);
    target = difficulty_adjustment_cash(target); //target += (target >> 2);
    static const uint256_t pow_limit(compact{ proof_of_work_limit });
    return target > pow_limit ? proof_of_work_limit : compact(target).normal();

}

// [CalculateNextWorkRequired]
uint32_t chain_state::work_required_retarget(const data& values)
{
    const compact bits(bits_high(values));
    
#ifdef LITECOIN
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

    return target > pow_limit ? proof_of_work_limit : compact(target).normal();


#else //LITECOIN
    BITCOIN_ASSERT_MSG(!bits.is_overflowed(), "previous block has bad bits");
	static const uint256_t pow_limit(compact{ proof_of_work_limit });    

    uint256_t target(bits);
    target *= retarget_timespan(values);
    target /= target_timespan_seconds;

    // The proof_of_work_limit constant is pre-normalized.
    return target > pow_limit ? proof_of_work_limit : compact(target).normal();
#endif //LITECOIN
}

// Get the bounded total time spanning the highest 2016 blocks.
uint32_t chain_state::retarget_timespan(const data& values)
{
    const auto high = timestamp_high(values);
    const auto retarget = values.timestamp.retarget;

    //*************************************************************************
    // CONSENSUS: subtract unsigned 32 bit numbers in signed 64 bit space in
    // order to prevent underflow before applying the range constraint.
    //*************************************************************************
    const auto timespan = cast_subtract<int64_t>(high, retarget);
    return range_constrain(timespan, min_timespan, max_timespan);
}

uint32_t chain_state::easy_work_required(const data& values, bool daa_active)
{
    BITCOIN_ASSERT(values.height != 0);

    // If the time limit has passed allow a minimum difficulty block.
    if (values.timestamp.self > easy_time_limit(values))
        return proof_of_work_limit;

    auto height = values.height;
    auto& bits = values.bits.ordered;

    if(daa_active) 
        return cash_difficulty_adjustment(values);
    else
    // Reverse iterate the ordered-by-height list of header bits.
        for (auto bit = bits.rbegin(); bit != bits.rend(); ++bit)
            if (is_retarget_or_non_limit(--height, *bit))
                return *bit;

    // Since the set of heights is either a full retarget range or ends at
    // zero this is not reachable unless the data set is invalid.
    BITCOIN_ASSERT(false);
    return proof_of_work_limit;
}

uint32_t chain_state::easy_time_limit(const chain_state::data& values)
{
    const int64_t high = timestamp_high(values);
    const int64_t spacing = easy_spacing_seconds;

    //*************************************************************************
    // CONSENSUS: add unsigned 32 bit numbers in signed 64 bit space in
    // order to prevent overflow before applying the domain constraint.
    //*************************************************************************
    return domain_constrain<uint32_t>(cast_add<int64_t>(high, spacing));
}

// A retarget height, or a block that does not have proof_of_work_limit bits.
bool chain_state::is_retarget_or_non_limit(size_t height, uint32_t bits)
{
    // Zero is a retarget height, ensuring termination before height underflow.
    // This is guaranteed, just asserting here to document the safeguard.
    BITCOIN_ASSERT_MSG(is_retarget_height(0), "loop overflow potential");

    return bits != proof_of_work_limit || is_retarget_height(height);
}

// Determine if height is a multiple of retargeting_interval.
bool chain_state::is_retarget_height(size_t height)
{
    return retarget_distance(height) == 0;
}

// Determine the number of blocks back to the closest retarget height.
size_t chain_state::retarget_distance(size_t height)
{
    return height % retargeting_interval;
}

// Publics.
//-----------------------------------------------------------------------------

// static
chain_state::map chain_state::get_map(size_t height,
    const checkpoints& checkpoints, uint32_t forks)
{
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

    return map;
}

// static
uint32_t chain_state::signal_version(uint32_t forks)
{
    if (script::is_enabled(forks, rule_fork::bip65_rule))
        return bip65_version;

    if (script::is_enabled(forks, rule_fork::bip66_rule))
        return bip66_version;

    if (script::is_enabled(forks, rule_fork::bip34_rule))
        return bip34_version;

    // Signal bip9 bit0 if any of the group is configured.
    if (script::is_enabled(forks, rule_fork::bip9_bit0_group))
        return bip9_version_base | bip9_version_bit0;

    return first_version;
}

// This is promotion from a preceding height to the next.
chain_state::data chain_state::to_pool(const chain_state& top)
{
    // Copy data from presumed previous-height block state.
    auto data = top.data_;

    // Alias configured forks, these don't change.
    const auto forks = top.forks_;

    // If this overflows height is zero and result is handled as invalid.
    const auto height = data.height + 1u;

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

    // If promoting from retarget height, move that timestamp into retarget.
    if (is_retarget_height(height - 1u))
        data.timestamp.retarget = data.timestamp.self;

    // Replace previous block state with tx pool chain state for next height.
    // Only height and version used by tx pool, others promotable or unused.
    // Preserve data.allow_collisions_hash promotion.
    // Preserve data.bip9_bit0_hash promotion.
    data.height = height;
    data.hash = null_hash;
    data.bits.self = proof_of_work_limit;
    data.version.self = signal_version(forks);
    data.timestamp.self = max_uint32;
    return data;
}

// Constructor (top to pool).
// This generates a state for the pool above the presumed top block state.
chain_state::chain_state(const chain_state& top)
  : data_(to_pool(top)),
    forks_(top.forks_),
    checkpoints_(top.checkpoints_),
    active_(activation(data_, forks_)),
    work_required_(work_required(data_, forks_)),
    median_time_past_(median_time_past(data_, forks_))
{
}

chain_state::data chain_state::to_block(const chain_state& pool,
    const block& block)
{
    auto testnet = script::is_enabled(pool.forks_, rule_fork::easy_blocks);

    // Copy data from presumed same-height pool state.
    auto data = pool.data_;

    // Replace pool chain state with block state at same (next) height.
    // Preserve data.timestamp.retarget promotion.
    const auto& header = block.header();
    data.hash = header.hash();
    data.bits.self = header.bits();
    data.version.self = header.version();
    data.timestamp.self = header.timestamp();

    // Cache hash of bip34 height block, otherwise use preceding state.
    if (allow_collisions(data.height, testnet))
        data.allow_collisions_hash = data.hash;

    // Cache hash of bip9 bit0 height block, otherwise use preceding state.
    if (bip9_bit0_active(data.height, testnet))
        data.bip9_bit0_hash = data.hash;

    return data;
}

// Constructor (tx pool to block).
// This assumes that the pool state is the same height as the block.
chain_state::chain_state(const chain_state& pool, const block& block)
  : data_(to_block(pool, block)),
    forks_(pool.forks_),
    checkpoints_(pool.checkpoints_),
    active_(activation(data_, forks_)),
    work_required_(work_required(data_, forks_)),
    median_time_past_(median_time_past(data_, forks_))
{
}

chain_state::data chain_state::to_header(const chain_state& parent,
    const header& header)
{
    auto testnet = script::is_enabled(parent.forks_, rule_fork::easy_blocks);

    // Copy and promote data from presumed parent-height header/block state.
    auto data = to_pool(parent);

    // Replace the pool (empty) current block state with given header state.
    // Preserve data.timestamp.retarget promotion.
    data.hash = header.hash();
    data.bits.self = header.bits();
    data.version.self = header.version();
    data.timestamp.self = header.timestamp();

    // Cache hash of bip34 height block, otherwise use preceding state.
    if (allow_collisions(data.height, testnet))
        data.allow_collisions_hash = data.hash;

    // Cache hash of bip9 bit0 height block, otherwise use preceding state.
    if (bip9_bit0_active(data.height, testnet))
        data.bip9_bit0_hash = data.hash;

    return data;
}

// Constructor (parent to header).
// This assumes that parent is the state of the header's previous block.
chain_state::chain_state(const chain_state& parent, const header& header)
  : data_(to_header(parent, header)),
    forks_(parent.forks_),
    checkpoints_(parent.checkpoints_),
    active_(activation(data_, forks_)),
    work_required_(work_required(data_, forks_)),
    median_time_past_(median_time_past(data_, forks_))
{
}

// Constructor (from raw data).
// The allow_collisions hard fork is always activated (not configurable).
chain_state::chain_state(data&& values, const checkpoints& checkpoints,
    uint32_t forks)
  : data_(std::move(values)),
    forks_(forks | rule_fork::allow_collisions),
    checkpoints_(checkpoints),
    active_(activation(data_, forks_)),
    work_required_(work_required(data_, forks_)),
    median_time_past_(median_time_past(data_, forks_))
{
}

// Semantic invalidity can also arise from too many/few values in the arrays.
// The same computations used to specify the ranges could detect such errors.
// These are the conditions that would cause exception during execution.
bool chain_state::is_valid() const
{
    return data_.height != 0;
}

// Properties.
//-----------------------------------------------------------------------------

size_t chain_state::height() const
{
    return data_.height;
}

uint32_t chain_state::enabled_forks() const
{
    return active_.forks;
}

uint32_t chain_state::minimum_version() const
{
    return active_.minimum_version;
}

uint32_t chain_state::median_time_past() const
{
    return median_time_past_;
}

uint32_t chain_state::work_required() const
{
    return work_required_;
}

// Forks.
//-----------------------------------------------------------------------------

bool chain_state::is_enabled(rule_fork fork) const
{
    return script::is_enabled(active_.forks, fork);
}

bool chain_state::is_checkpoint_conflict(const hash_digest& hash) const
{
    return !checkpoint::validate(hash, data_.height, checkpoints_);
}

bool chain_state::is_under_checkpoint() const
{
    // This assumes that the checkpoints are sorted.
    return checkpoint::covered(data_.height, checkpoints_);
}

// Mining.
//-----------------------------------------------------------------------------

uint32_t chain_state::get_next_work_required(uint32_t time_now)
{
    auto values = this->data_;
    values.timestamp.self = time_now;
    return work_required(values, this->enabled_forks());

}

} // namespace chain
} // namespace libbitcoin
