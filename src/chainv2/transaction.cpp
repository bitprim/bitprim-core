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
#include <bitcoin/bitcoin/chainv2/transaction.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <type_traits>
#include <sstream>
#include <type_traits>
#include <utility>
#include <vector>

#include <boost/optional.hpp>

#include <bitcoin/bitcoin/chain/chain_state.hpp>

#include <bitcoin/bitcoin/chainv2/input.hpp>
#include <bitcoin/bitcoin/chainv2/output.hpp>
#include <bitcoin/bitcoin/chainv2/script.hpp>

#include <bitcoin/bitcoin/constants.hpp>
#include <bitcoin/bitcoin/error.hpp>
#include <bitcoin/bitcoin/math/limits.hpp>
#include <bitcoin/bitcoin/machine/opcode.hpp>
#include <bitcoin/bitcoin/machine/operation.hpp>
#include <bitcoin/bitcoin/machine/rule_fork.hpp>
#include <bitcoin/bitcoin/message/messages.hpp>
#include <bitcoin/bitcoin/multi_crypto_support.hpp>
#include <bitcoin/bitcoin/utility/collection.hpp>
#include <bitcoin/bitcoin/utility/container_sink.hpp>
#include <bitcoin/bitcoin/utility/container_source.hpp>
#include <bitcoin/bitcoin/utility/endian.hpp>
#include <bitcoin/bitcoin/utility/istream_reader.hpp>
#include <bitcoin/bitcoin/utility/ostream_writer.hpp>


namespace libbitcoin { namespace chainv2 {

static_assert(std::is_move_constructible<transaction>::value, "std::is_move_constructible<transaction>::value");
static_assert(std::is_nothrow_move_constructible<transaction>::value, "std::is_nothrow_move_constructible<transaction>::value");
static_assert(std::is_move_assignable<transaction>::value, "std::is_move_assignable<transaction>::value");
static_assert(std::is_nothrow_move_assignable<transaction>::value, "std::is_nothrow_move_assignable<transaction>::value");
static_assert(std::is_copy_constructible<transaction>::value, "std::is_copy_constructible<transaction>::value");
static_assert(std::is_copy_assignable<transaction>::value, "std::is_copy_assignable<transaction>::value");


using bc::machine::rule_fork;

// Read a length-prefixed collection of inputs or outputs from the source.
template<class Source, class Put>
bool read_inputs(Source& source, std::vector<Put>& puts) {
    auto result = true;
    auto const count = source.read_size_little_endian();

    // Guard against potential for arbitary memory allocation.
    if (count > get_max_block_size()) {
        source.invalidate();
    } else {
        puts.resize(count);
    }

    auto const deserialize = [&result, &source](Put& put) {
        result = result && put.from_data(source, true);
    };

    std::for_each(puts.begin(), puts.end(), deserialize);
    return result;
}

// Write a length-prefixed collection of inputs or outputs to the sink.
template<class Sink, class Put>
void write(Sink& sink, const std::vector<Put>& puts, bool wire) {
    sink.write_variable_little_endian(puts.size());

    auto const serialize = [&sink, wire](const Put& put) {
        put.to_data(sink, wire);
    };

    std::for_each(puts.begin(), puts.end(), serialize);
}

// Constructors.
//-----------------------------------------------------------------------------

transaction::transaction()
    : version_(0), locktime_(0)
{}

// transaction::transaction(uint32_t version, uint32_t locktime, input::list&& inputs) //, output::list&& outputs)
//     : version_(version)
//     , locktime_(locktime)
//     , inputs_(std::move(inputs))
//     // , outputs_(std::move(outputs))
// {}

// transaction::transaction(uint32_t version, uint32_t locktime, input::list const& inputs) //, output::list const& outputs)
//     : version_(version)
//     , locktime_(locktime)
//     , inputs_(inputs)
//     // , outputs_(outputs)
// {}

// Operators.
//-----------------------------------------------------------------------------

// friend
bool operator==(transaction const& a, transaction const& b) {
    return (a.version_ == b.version_)
        && (a.locktime_ == b.locktime_)
        && (a.inputs_ == b.inputs_)
        // && (a.outputs_ == b.outputs_);
        && (a.outputs_info_ == b.outputs_info_);
}

// friend
bool operator!=(transaction const& a, transaction const& b) {
    return !(a == b);
}

// Deserialization.
//-----------------------------------------------------------------------------

// static
transaction transaction::factory_from_data(data_chunk&& data, uint64_t minimum_output_satoshis) {
    transaction instance;
    instance.from_data(std::move(data), minimum_output_satoshis);
    return instance;
}

// static
transaction transaction::factory_from_data(data_chunk const& data, uint64_t minimum_output_satoshis) {
    transaction instance;
    instance.from_data(data, minimum_output_satoshis);
    return instance;
}

// static
// protected
transaction transaction::factory_from_data(std::istream& stream, uint64_t minimum_output_satoshis) {
    transaction instance;
    instance.from_data(stream, minimum_output_satoshis);
    return instance;
}

// static
// protected
transaction transaction::factory_from_data(reader& source, uint64_t minimum_output_satoshis) {
    transaction instance;
    instance.from_data(source, minimum_output_satoshis);
    return instance;
}

// static
bool transaction::from_data(data_chunk&& data, uint64_t minimum_output_satoshis) {
    data_ = std::move(data);
    data_source istream(data_);
    return from_data(istream, minimum_output_satoshis);
}

// static
bool transaction::from_data(data_chunk const& data, uint64_t minimum_output_satoshis) {
    data_ = data;
    data_source istream(data_);
    return from_data(istream, minimum_output_satoshis);
}

// protected
bool transaction::from_data(std::istream& stream, uint64_t minimum_output_satoshis) {
    istream_reader source(stream);
    return from_data(source, minimum_output_satoshis);
}


inline
size_t opcode_value(machine::opcode code) {
    if (code == machine::opcode::checksig || code == machine::opcode::checksigverify) {
        // ++total;
        return 1;
    } 
    
    if (code == machine::opcode::checkmultisig || code == machine::opcode::checkmultisigverify) {
        // total += multisig_default_sigops; //ops(false, preceding);
        return multisig_default_sigops;
    }

    return 0;
}



// bool script::is_unspendable() const {
//     // The first operations access must be method-based to guarantee the cache.
//     return (!operations().empty() && operations_[0].code() == opcode::return_)
//         || satoshi_content_size() > max_script_size;
// }

/*
// Concurrent read/write is not supported, so no critical section.
bool script::from_data(reader& source, bool prefix)
{
    reset();
    valid_ = true;

    if (prefix)
    {
        const auto size = source.read_size_little_endian();

        // The max_script_size constant limits evaluation, but not all scripts
        // evaluate, so use max_block_size to guard memory allocation here.
        if (size > get_max_block_size())
            source.invalidate();
        else
            bytes_ = source.read_bytes(size);
    }
    else
    {
        bytes_ = source.read_bytes();
    }

    if (!source)
        reset();

    return source;
}

*/

inline
std::tuple<size_t, bool, bool> count_output_sigops_is_unspendable(reader& source) {
    size_t total = 0;
    bool is_unspendable = false;

    std::cout << "transaction::count_output_sigops_is_unspendable - 1 - bool(source): " << bool(source) << std::endl;

    auto const satoshi_content_size = source.read_size_little_endian();

    std::cout << "transaction::count_output_sigops_is_unspendable - 2 - bool(source): " << bool(source) << std::endl;


    if (satoshi_content_size > get_max_block_size()) {
        source.invalidate();
        std::cout << "transaction::count_output_sigops_is_unspendable - 3 - bool(source): " << bool(source) << std::endl;
        
        return std::make_tuple(total, is_unspendable, false);
    }

    std::cout << "transaction::count_output_sigops_is_unspendable - 4 - bool(source): " << bool(source) << std::endl;

    // if ( ! source.is_exhausted() && satoshi_content_size > 0) {
    if ( source && satoshi_content_size > 0) {
        std::cout << "transaction::count_output_sigops_is_unspendable - 5 - bool(source): " << bool(source) << std::endl;

        auto const op = machine::operation::factory_from_data(source);
    
        std::cout << "transaction::count_output_sigops_is_unspendable - 6 - bool(source): " << bool(source) << std::endl;

        machine::opcode const code = op.code();
        total += opcode_value(code);

        std::cout << "transaction::count_output_sigops_is_unspendable - 7 - bool(source): " << bool(source) << std::endl;

        if (code == machine::opcode::return_) {
            is_unspendable = true;
        }
    }

    size_t n = 1;
    // while ( ! source.is_exhausted() && n < satoshi_content_size) {
    while ( source && n < satoshi_content_size) {
        std::cout << "transaction::count_output_sigops_is_unspendable - 8 - bool(source): " << bool(source) << std::endl;
        auto const op = machine::operation::factory_from_data(source);
        std::cout << "transaction::count_output_sigops_is_unspendable - 9 - bool(source): " << bool(source) << std::endl;

        machine::opcode const code = op.code();
        total += opcode_value(code);
        ++n;
    }

    std::cout << "transaction::count_output_sigops_is_unspendable - 10 - bool(source): " << bool(source) << std::endl;

    is_unspendable = is_unspendable || satoshi_content_size > max_script_size;
    return std::make_tuple(total, is_unspendable, source);
}

// bool output::is_dust(uint64_t minimum_output_value) const {
//     // If provably unspendable it does not expand the unspent output set.
//     return value_ < minimum_output_value && !script_.is_unspendable();
// }


/*


bool output::from_data(reader& source, bool wire)
{
    reset();

    if (!wire)
        validation.spender_height = source.read_4_bytes_little_endian();

    value_ = source.read_8_bytes_little_endian();
    script_.from_data(source, true);

    if (!source)
        reset();

    return source;
}


// Read a length-prefixed collection of inputs or outputs from the source.
template<class Source, class Put>
bool read(Source& source, std::vector<Put>& puts, bool wire)
{
    auto result = true;
    const auto count = source.read_size_little_endian();

    // Guard against potential for arbitary memory allocation.
    if (count > get_max_block_size())
        source.invalidate();
    else
        puts.resize(count);

    const auto deserialize = [&result, &source, wire](Put& put)
    {
        result = result && put.from_data(source, wire);
    };

    std::for_each(puts.begin(), puts.end(), deserialize);
    return result;
}
*/

bool transaction::read_outputs_info(reader& source, uint64_t minimum_output_satoshis) {
    std::cout << "transaction::read_outputs_info - 1 - bool(source): " << bool(source) << std::endl;
    outputs_info_.total_output_value = 0;
    outputs_info_.signature_operations = 0;
    outputs_info_.any_is_dusty = false;
    outputs_info_.count = source.read_size_little_endian();


    std::cout << "transaction::read_outputs_info - 2 - bool(source): " << bool(source) << std::endl;

    // Guard against potential for arbitary memory allocation.
    if (outputs_info_.count > get_max_block_size()) {
        source.invalidate();

        std::cout << "transaction::read_outputs_info - 3 - bool(source): " << bool(source) << std::endl;
        return false;
    } 

    std::cout << "transaction::read_outputs_info - 4 - bool(source): " << bool(source) << std::endl;
    

    size_t i = 0;
    while (source && i < outputs_info_.count) {
            std::cout << "transaction::read_outputs_info - 5 - bool(source): " << bool(source) << std::endl;

        auto const value = source.read_8_bytes_little_endian();

        std::cout << "transaction::read_outputs_info - 6 - bool(source): " << bool(source) << std::endl;

        auto const res = count_output_sigops_is_unspendable(source);

        std::cout << "transaction::read_outputs_info - 7 - bool(source): " << bool(source) << std::endl;


        if (! std::get<2>(res)) {
            std::cout << "transaction::read_outputs_info - 8 - bool(source): " << bool(source) << std::endl;
            return false;
        }

        std::cout << "transaction::read_outputs_info - 9 - bool(source): " << bool(source) << std::endl;


        outputs_info_.total_output_value += value;
        outputs_info_.signature_operations += std::get<0>(res);
        bool const script_is_unspendable = std::get<1>(res);

        std::cout << "transaction::read_outputs_info - 10 - bool(source): " << bool(source) << std::endl;

        if ( ! outputs_info_.any_is_dusty && value < minimum_output_satoshis && !script_is_unspendable) {
            outputs_info_.any_is_dusty = true;
        }

        ++i;
    }

    std::cout << "transaction::read_outputs_info - 11 - bool(source): " << bool(source) << std::endl;

    return source;    
}


/*
version_ = source.read_4_bytes_little_endian();
read(source, inputs_, wire);
read(source, outputs_, wire);
locktime_ = source.read_4_bytes_little_endian();
*/

// protected
bool transaction::from_data(reader& source, uint64_t minimum_output_satoshis) {
    reset();

    // Wire (satoshi protocol) deserialization.
    version_ = source.read_4_bytes_little_endian();

    std::cout << "1 transaction::from_data - inputs_.size(): " << inputs_.size() << " - bool(source): " << bool(source) << std::endl;
    read_inputs(source, inputs_);
    std::cout << "2 transaction::from_data - inputs_.size(): " << inputs_.size() << " - bool(source): " << bool(source) << std::endl;

    // && read_output(source, outputs_, wire); //TODO(fernando)
    read_outputs_info(source, minimum_output_satoshis);

    std::cout << "3 transaction::from_data - inputs_.size(): " << inputs_.size() << " - bool(source): " << bool(source) << std::endl;

    locktime_ = source.read_4_bytes_little_endian();


    std::cout << "4 transaction::from_data - inputs_.size(): " << inputs_.size() << " - bool(source): " << bool(source) << std::endl;

    if (!source) {
        std::cout << "5 transaction::from_data - inputs_.size(): " << inputs_.size() << " - bool(source): " << bool(source) << std::endl;
        reset();
    }

    std::cout << "6 transaction::from_data - inputs_.size(): " << inputs_.size() << " - bool(source): " << bool(source) << std::endl;


    return source;
}

// protected
void transaction::reset() {
    version_ = 0;
    locktime_ = 0;
    inputs_.clear();
    inputs_.shrink_to_fit();
    // outputs_.clear();
    // outputs_.shrink_to_fit();
    // invalidate_cache();
    // total_input_value_ = boost::none;
    // total_output_value_ = boost::none;

    outputs_info_.total_output_value = 0;
    outputs_info_.signature_operations = 0;
    outputs_info_.any_is_dusty = false;
    outputs_info_.count = 0;
}

bool transaction::is_valid() const {
    // return (version_ != 0) || (locktime_ != 0) || ! inputs_.empty() || ! outputs_.empty();
    return (version_ != 0) || (locktime_ != 0) || ! inputs_.empty() || outputs_info_.count > 0;
}


data_chunk const& transaction::data() const {
    return data_;
}

// Serialization.
//-----------------------------------------------------------------------------

// data_chunk transaction::to_data(bool wire) const {
//     data_chunk data;
//     auto const size = serialized_size(wire);

//     // Reserve an extra byte to prevent full reallocation in the case of
//     // generate_signature_hash extension by addition of the sighash_type.
//     data.reserve(size + sizeof(uint8_t));

//     data_sink ostream(data);
//     to_data(ostream, wire);
//     ostream.flush();
//     BITCOIN_ASSERT(data.size() == size);
//     return data;
// }

// void transaction::to_data(std::ostream& stream, bool wire) const {
//     ostream_writer sink(stream);
//     to_data(sink, wire);
// }

// void transaction::to_data(writer& sink, bool wire) const {
//     if (wire) {
//         // Wire (satoshi protocol) serialization.
//         sink.write_4_bytes_little_endian(version_);
//         write(sink, inputs_, wire);
//         write(sink, outputs_, wire);
//         sink.write_4_bytes_little_endian(locktime_);
//     } else {
//         // Database (outputs forward) serialization.
//         write(sink, outputs_, wire);
//         write(sink, inputs_, wire);
//         sink.write_variable_little_endian(locktime_);
//         sink.write_variable_little_endian(version_);
//     }
// }

// Size.
//-----------------------------------------------------------------------------

// size_t transaction::serialized_size(bool wire) const {
//     auto const ins = [wire](size_t size, input const& input) {
//         return size + input.serialized_size(wire);
//     };

//     auto const outs = [wire](size_t size, output const& output) {
//         return size + output.serialized_size(wire);
//     };

//     return (wire ? sizeof(version_) : message::variable_uint_size(version_))
//         + (wire ? sizeof(locktime_) : message::variable_uint_size(locktime_))
//         + message::variable_uint_size(inputs_.size())
//         + message::variable_uint_size(outputs_.size())
//         + std::accumulate(inputs_.begin(), inputs_.end(), size_t{0}, ins)
//         + std::accumulate(outputs_.begin(), outputs_.end(), size_t{0}, outs)
//         ;
// }

size_t transaction::serialized_size_wired() const {
    return serialized_size_wired_;
}

// Accessors.
//-----------------------------------------------------------------------------

uint32_t transaction::version() const {
    return version_;
}

void transaction::set_version(uint32_t value) {
    version_ = value;
    // invalidate_cache();
}

uint32_t transaction::locktime() const {
    return locktime_;
}

void transaction::set_locktime(uint32_t value) {
    locktime_ = value;
    // invalidate_cache();
}


input::list& transaction::inputs() {
    return inputs_;
}

input::list const& transaction::inputs() const {
    return inputs_;
}

void transaction::set_inputs(input::list const& value) {
    inputs_ = value;
    // invalidate_cache();
    // total_input_value_ = boost::none;
}

void transaction::set_inputs(input::list&& value) {
    inputs_ = std::move(value);
    // invalidate_cache();
    // total_input_value_ = boost::none;
}

chainv2::outputs_info const& transaction::outputs_info() const {
    return outputs_info_;
}

// output::list& transaction::outputs() {
//     return outputs_;
// }

// output::list const& transaction::outputs() const {
//     return outputs_;
// }

// void transaction::set_outputs(output::list const& value) {
//     outputs_ = value;
//     // invalidate_cache();
//     // total_output_value_ = boost::none;
// }

// void transaction::set_outputs(output::list&& value) {
//     outputs_ = std::move(value);
//     // invalidate_cache();
//     // total_output_value_ = boost::none;
// }

// Cache.
//-----------------------------------------------------------------------------

// void transaction::invalidate_cache() const {
//     ///////////////////////////////////////////////////////////////////////////
//     // Critical Section
//     mutex_.lock_upgrade();

//     if (hash_) {
//         mutex_.unlock_upgrade_and_lock();
//         //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//         hash_.reset();
//         //---------------------------------------------------------------------
//         mutex_.unlock_and_lock_upgrade();
//     }

//     mutex_.unlock_upgrade();
//     ///////////////////////////////////////////////////////////////////////////
// }

hash_digest transaction::hash() const {
    return bitcoin_hash(data());
}

// hash_digest transaction::hash() const {
//     return bitcoin_hash(to_data(true));
//     // ///////////////////////////////////////////////////////////////////////////
//     // // Critical Section
//     // mutex_.lock_upgrade();

//     // if (!hash_) {
//     //     //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//     //     mutex_.unlock_upgrade_and_lock();
//     //     hash_ = std::make_shared<hash_digest>(bitcoin_hash(to_data(true)));
//     //     mutex_.unlock_and_lock_upgrade();
//     //     //---------------------------------------------------------------------
//     // }

//     // auto const hash = *hash_;
//     // mutex_.unlock_upgrade();
//     // ///////////////////////////////////////////////////////////////////////////

//     // return hash;
// }

// hash_digest transaction::hash(uint32_t sighash_type) const {
//     auto serialized = to_data(true);
//     extend_data(serialized, to_little_endian(sighash_type));
//     return bitcoin_hash(serialized);
// }

// void transaction::recompute_hash() {
//     hash_ = nullptr;
//     hash();
// }

// Validation helpers.
//-----------------------------------------------------------------------------

bool transaction::is_coinbase() const {
    return inputs_.size() == 1 && inputs_.front().previous_output().is_null();
}

// True if coinbase and has invalid input[0] script size.
bool transaction::is_oversized_coinbase() const {
    if (!is_coinbase()) {
        return false;
    }

    auto const script_size = inputs_.front().script().serialized_size(false);
    return script_size < min_coinbase_size || script_size > max_coinbase_size;
}

// True if not coinbase but has null previous_output(s).
bool transaction::is_null_non_coinbase() const{
    if (is_coinbase()) {
        return false;
    }

    auto const invalid = [](input const& input) {
        return input.previous_output().is_null();
    };

    return std::any_of(inputs_.begin(), inputs_.end(), invalid);
}

// private
bool transaction::all_inputs_final() const {
    auto const finalized = [](input const& input) {
        return input.is_final();
    };

    return std::all_of(inputs_.begin(), inputs_.end(), finalized);
}

bool transaction::is_final(size_t block_height, uint32_t block_time) const {
    auto const max_locktime = [=]() {
        return locktime_ < locktime_threshold ?
            safe_unsigned<uint32_t>(block_height) : block_time;
    };

    return locktime_ == 0 || locktime_ < max_locktime() || all_inputs_final();
}

bool transaction::is_locked(size_t block_height, uint32_t median_time_past) const {
    if (version_ < relative_locktime_min_version || is_coinbase()) {
        return false;
    }

    auto const locked = [block_height, median_time_past](input const& input) {
        return input.is_locked(block_height, median_time_past);
    };

    // If any input is relative time locked the transaction is as well.
    return std::any_of(inputs_.begin(), inputs_.end(), locked);
}

// This is not a consensus rule, just detection of an irrational use.
bool transaction::is_locktime_conflict() const {
    return locktime_ != 0 && all_inputs_final();
}

// // Returns max_uint64 in case of overflow.
// uint64_t transaction::total_input_value() const {
//     uint64_t value;

//     ///////////////////////////////////////////////////////////////////////////
//     // Critical Section
//     mutex_.lock_upgrade();

//     if (total_input_value_ != boost::none) {
//         value = total_input_value_.get();
//         mutex_.unlock_upgrade();
//         //---------------------------------------------------------------------
//         return value;
//     }

//     mutex_.unlock_upgrade_and_lock();
//     //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//     ////static_assert(max_money() < max_uint64, "overflow sentinel invalid");
//     auto const sum = [](uint64_t total, input const& input) {
//         auto const& prevout = input.previous_output().validation.cache;
//         auto const missing = !prevout.is_valid();

//         // Treat missing previous outputs as zero-valued, no math on sentinel.
//         return ceiling_add(total, missing ? 0 : prevout.value());
//     };

//     value = std::accumulate(inputs_.begin(), inputs_.end(), uint64_t(0), sum);
//     total_input_value_ = value;
//     mutex_.unlock();
//     ///////////////////////////////////////////////////////////////////////////

//     return value;
// }

// Returns max_uint64 in case of overflow.
uint64_t transaction::total_input_value() const {
    ////static_assert(max_money() < max_uint64, "overflow sentinel invalid");
    auto const sum = [](uint64_t total, input const& input) {
        auto const& prevout = input.previous_output().validation.cache;
        auto const missing = !prevout.is_valid();

        // Treat missing previous outputs as zero-valued, no math on sentinel.
        return ceiling_add(total, missing ? 0 : prevout.value());
    };

    return std::accumulate(inputs_.begin(), inputs_.end(), uint64_t(0), sum);
}

// // Returns max_uint64 in case of overflow.
// uint64_t transaction::total_output_value() const {
//     uint64_t value;

//     ///////////////////////////////////////////////////////////////////////////
//     // Critical Section
//     mutex_.lock_upgrade();

//     if (total_output_value_ != boost::none) {
//         value = total_output_value_.get();
//         mutex_.unlock_upgrade();
//         //---------------------------------------------------------------------
//         return value;
//     }

//     mutex_.unlock_upgrade_and_lock();
//     //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//     ////static_assert(max_money() < max_uint64, "overflow sentinel invalid");
//     auto const sum = [](uint64_t total, output const& output) {
//         return ceiling_add(total, output.value());
//     };

//     value = std::accumulate(outputs_.begin(), outputs_.end(), uint64_t(0), sum);
//     total_output_value_ = value;
//     mutex_.unlock();
//     ///////////////////////////////////////////////////////////////////////////

//     return value;
// }

// Returns max_uint64 in case of overflow.
uint64_t transaction::total_output_value() const {
    // ////static_assert(max_money() < max_uint64, "overflow sentinel invalid");
    // auto const sum = [](uint64_t total, output const& output) {
    //     return ceiling_add(total, output.value());
    // };

    // return std::accumulate(outputs_.begin(), outputs_.end(), uint64_t(0), sum);

    return outputs_info_.total_output_value;
}

uint64_t transaction::fees() const {
    return floor_subtract(total_input_value(), total_output_value());
}

bool transaction::is_overspent() const {
    return !is_coinbase() && total_output_value() > total_input_value();
}

// // Returns max_size_t in case of overflow.
// size_t transaction::signature_operations() const {
//     auto const state = validation.state;
//     return state ? signature_operations(state->is_enabled(rule_fork::bip16_rule)) : max_size_t;
// }

// Returns max_size_t in case of overflow.
size_t transaction::signature_operations(bool bip16_active) const {
    auto const in = [bip16_active](size_t total, input const& input) {
        // This includes BIP16 p2sh additional sigops if prevout is cached.
        return ceiling_add(total, input.signature_operations(bip16_active));
    };

    // auto const out = [](size_t total, output const& output) {
    //     return ceiling_add(total, output.signature_operations());
    // };

    // return std::accumulate(inputs_.begin(), inputs_.end(), size_t{0}, in) +
    //     std::accumulate(outputs_.begin(), outputs_.end(), size_t{0}, out);

    return std::accumulate(inputs_.begin(), inputs_.end(), size_t{0}, in) +
        outputs_info_.signature_operations;

}

bool transaction::is_missing_previous_outputs() const {
    auto const missing = [](input const& input) {
        auto const& prevout = input.previous_output();
        auto const coinbase = prevout.is_null();
        auto const missing = !prevout.validation.cache.is_valid();
        return missing && !coinbase;
    };

    // This is an optimization of !missing_inputs().empty();
    return std::any_of(inputs_.begin(), inputs_.end(), missing);
}

point::list transaction::previous_outputs() const {
    point::list prevouts(inputs_.size());

    auto const pointer = [](input const& input)
    {
        return input.previous_output();
    };

    auto const& ins = inputs_;
    std::transform(ins.begin(), ins.end(), prevouts.begin(), pointer);
    return prevouts;
}

point::list transaction::missing_previous_outputs() const {
    point::list prevouts;

    for (auto& input: inputs_) {
        auto const& prevout = input.previous_output();
        auto const missing = !prevout.validation.cache.is_valid();

        if (missing && !prevout.is_null()) {
            prevouts.push_back(prevout);
        }
    }

    return prevouts;
}

hash_list transaction::missing_previous_transactions() const {
    auto const points = missing_previous_outputs();
    hash_list hashes(points.size());

    // auto const hasher = [](output_point const& point) { return point.hash(); };
    auto const hasher = [](point const& point) { return point.hash(); };

    std::transform(points.begin(), points.end(), hashes.begin(), hasher);
    return distinct(hashes);
}

bool transaction::is_internal_double_spend() const {
    auto prevouts = previous_outputs();
    std::sort(prevouts.begin(), prevouts.end());
    auto const distinct_end = std::unique(prevouts.begin(), prevouts.end());
    auto const distinct = (distinct_end == prevouts.end());
    return !distinct;
}

bool transaction::is_double_spend(bool include_unconfirmed) const {
    auto const spent = [include_unconfirmed](input const& input) {
        auto const& prevout = input.previous_output().validation;
        return prevout.spent && (include_unconfirmed || prevout.confirmed);
    };

    return std::any_of(inputs_.begin(), inputs_.end(), spent);
}

bool transaction::is_dusty(uint64_t minimum_output_value) const {
    // auto const dust = [minimum_output_value](output const& output) {
    //     return output.is_dust(minimum_output_value);
    // };

    // return std::any_of(outputs_.begin(), outputs_.end(), dust);

    return outputs_info_.any_is_dusty;
}

bool transaction::is_mature(size_t height) const {
    auto const mature = [height](input const& input) {
        return input.previous_output().is_mature(height);
    };

    return std::all_of(inputs_.begin(), inputs_.end(), mature);
}

// // Coinbase transactions return success, to simplify iteration.
// code transaction::connect_input(chain::chain_state const& state, size_t input_index) const {
//     if (input_index >= inputs_.size()) {
//         return error::operation_failed;
//     }

//     if (is_coinbase()) {
//         return error::success;
//     }

//     auto const& prevout = inputs_[input_index].previous_output().validation;

//     // Verify that the previous output cache has been populated.
//     if (!prevout.cache.is_valid()) {
//         return error::missing_previous_output;
//     }

//     auto const forks = state.enabled_forks();
//     auto const index32 = static_cast<uint32_t>(input_index);

//     // Verify the transaction input script against the previous output.
//     return script::verify(*this, index32, forks);
// }

// Validation.
//-----------------------------------------------------------------------------

// These checks are self-contained; blockchain (and so version) independent.
code transaction::check(bool transaction_pool) const {
    // if (inputs_.empty() || outputs_.empty()) {
    if (inputs_.empty() || outputs_info_.count == 0) {
        return error::empty_transaction;
    } 
    
    if (is_null_non_coinbase()) {
        return error::previous_output_null;
    } 
    
    if (total_output_value() > max_money()) {
        return error::spend_overflow;
    } 
    
    if (!transaction_pool && is_oversized_coinbase()) {
        return error::invalid_coinbase_script_size;
    } 
    
    if (transaction_pool && is_coinbase()) {
        return error::coinbase_transaction;
    } 
    
    if (transaction_pool && is_internal_double_spend()) {
        return error::transaction_internal_double_spend;
    } 
    
    // if (transaction_pool && serialized_size(true) >= get_max_block_size()) {
    if (transaction_pool && serialized_size_wired() >= get_max_block_size()) {
        return error::transaction_size_limit;
    }

    // We cannot know if bip16 is enabled at this point so we disable it.
    // This will not make a difference unless prevouts are populated, in which
    // case they are ignored. This means that p2sh sigops are not counted here.
    // This is a preliminary check, the final count must come from accept().
    // Reenable once sigop caching is implemented, otherwise is deoptimization.
    ////else if (transaction_pool && signature_operations(false) > get_max_block_sigops()
    ////    return error::transaction_legacy_sigop_limit;

    return error::success;
}

// code transaction::accept(bool transaction_pool) const {
//     auto const state = validation.state;
//     return state ? accept(*state, transaction_pool) : error::operation_failed;
// }

// These checks assume that prevout caching is completed on all tx.inputs.
code transaction::accept(chain::chain_state const& state, bool tx_duplicate, bool transaction_pool) const {
    auto const bip16 = state.is_enabled(rule_fork::bip16_rule);
    auto const bip30 = state.is_enabled(rule_fork::bip30_rule);
    auto const bip68 = state.is_enabled(rule_fork::bip68_rule);

    // We don't need to allow tx pool acceptance of an unspent duplicate
    // because tx pool inclusion cannot be required by consensus.
    auto const duplicates = state.is_enabled(rule_fork::allow_collisions) && !transaction_pool;

    if (transaction_pool && state.is_under_checkpoint()) {
        return error::premature_validation;
    }

    if (transaction_pool && !is_final(state.height(), state.median_time_past())) {
        return error::transaction_non_final;
    } 

    //*************************************************************************
    // CONSENSUS:
    // A transaction hash that exists in the chain is not acceptable even if
    // the original is spent in the new block. This is not necessary nor is it
    // described by BIP30, but it is in the code referenced by BIP30. As such
    // the tx pool need only test against the chain, skipping the pool.
    //*************************************************************************
    // if (!duplicates && bip30 && validation.duplicate) {
    //     return error::unspent_duplicate;
    // } 

    if (!duplicates && bip30 && tx_duplicate) {
        return error::unspent_duplicate;
    }

    if (is_missing_previous_outputs()) {
        return error::missing_previous_output;
    } 
    
    if (is_double_spend(transaction_pool)) {
        return error::double_spend;
    } 
    
    // This relates height to maturity of spent coinbase. Since reorg is the
    // only way to decrease height and reorg invalidates, this is cache safe.
    if (!is_mature(state.height())) {
        return error::coinbase_maturity;
    } 
    
    if (is_overspent()) {
        return error::spend_exceeds_value;
    } 
    
    if (bip68 && is_locked(state.height(), state.median_time_past())) {
        return error::sequence_locked;
    } 
    
    // This recomputes sigops to include p2sh from prevouts if bip16 is true.
    if (transaction_pool && signature_operations(bip16) > get_max_block_sigops()) {
        return error::transaction_embedded_sigop_limit;
    } 

    return error::success;
}

// code transaction::connect() const {
//     auto const state = validation.state;
//     return state ? connect(*state) : error::operation_failed;
// }

// code transaction::connect(chain::chain_state const& state) const {
//     code ec;

//     for (size_t input = 0; input < inputs_.size(); ++input) {
//         if ((ec = connect_input(state, input))) {
//             return ec;
//         }
//     }

//     return error::success;
// }

//Note(fernando): Not necessary for Tx validation
// bool transaction::is_standard() const {
//     for (auto const& in : inputs()) {
//         if ( in.script().pattern() == libbitcoin::machine::script_pattern::non_standard){
//             return false;
//         }
//     }

//     for (auto const& out : outputs()) {
//         if ( out.script().pattern() == libbitcoin::machine::script_pattern::non_standard){
//             return false;
//         }
//     }
//     return true;
// }

}} // namespace libbitcoin::chainv2

