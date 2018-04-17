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
#ifndef LIBBITCOIN_MACHINE_INTERPRETER_HPP
#define LIBBITCOIN_MACHINE_INTERPRETER_HPP

#include <cstdint>
#include <bitcoin/bitcoin/define.hpp>
#include <bitcoin/bitcoin/error.hpp>
#include <bitcoin/bitcoin/machine/opcode.hpp>
#include <bitcoin/bitcoin/machine/operation.hpp>
#include <bitcoin/bitcoin/machine/program.hpp>
#include <bitcoin/bitcoin/utility/data.hpp>

namespace libbitcoin {
namespace machine {

template <typename Program>
class BC_API interpreter {
public:
    typedef error::error_code_t result;

    // Operations (shared).
    //-----------------------------------------------------------------------------

    static result op_nop(opcode);
    static result op_disabled(opcode);
    static result op_reserved(opcode);
    static result op_push_number(Program& program, uint8_t value);
    static result op_push_size(Program& program, const operation& op);
    static result op_push_data(Program& program, const data_chunk& data,
        uint32_t size_limit);

    // Operations (not shared).
    //-----------------------------------------------------------------------------

    static result op_if(Program& program);
    static result op_notif(Program& program);
    static result op_else(Program& program);
    static result op_endif(Program& program);
    static result op_verify(Program& program);
    static result op_return(Program& program);
    static result op_to_alt_stack(Program& program);
    static result op_from_alt_stack(Program& program);
    static result op_drop2(Program& program);
    static result op_dup2(Program& program);
    static result op_dup3(Program& program);
    static result op_over2(Program& program);
    static result op_rot2(Program& program);
    static result op_swap2(Program& program);
    static result op_if_dup(Program& program);
    static result op_depth(Program& program);
    static result op_drop(Program& program);
    static result op_dup(Program& program);
    static result op_nip(Program& program);
    static result op_over(Program& program);
    static result op_pick(Program& program);
    static result op_roll(Program& program);
    static result op_rot(Program& program);
    static result op_swap(Program& program);
    static result op_tuck(Program& program);
    static result op_size(Program& program);
    static result op_equal(Program& program);
    static result op_equal_verify(Program& program);
    static result op_add1(Program& program);
    static result op_sub1(Program& program);
    static result op_negate(Program& program);
    static result op_abs(Program& program);
    static result op_not(Program& program);
    static result op_nonzero(Program& program);
    static result op_add(Program& program);
    static result op_sub(Program& program);
    static result op_bool_and(Program& program);
    static result op_bool_or(Program& program);
    static result op_num_equal(Program& program);
    static result op_num_equal_verify(Program& program);
    static result op_num_not_equal(Program& program);
    static result op_less_than(Program& program);
    static result op_greater_than(Program& program);
    static result op_less_than_or_equal(Program& program);
    static result op_greater_than_or_equal(Program& program);
    static result op_min(Program& program);
    static result op_max(Program& program);
    static result op_within(Program& program);
    static result op_ripemd160(Program& program);
    static result op_sha1(Program& program);
    static result op_sha256(Program& program);
    static result op_hash160(Program& program);
    static result op_hash256(Program& program);
    static result op_codeseparator(Program& program, const operation& op);
    static result op_check_sig_verify(Program& program);
    static result op_check_sig(Program& program);
    static result op_check_multisig_verify(Program& program);
    static result op_check_multisig(Program& program);
    static result op_check_locktime_verify(Program& program);
    static result op_check_sequence_verify(Program& program);

    /// Run program script.
    static code run(Program& program);

    /// Run individual operations (idependent of the script).
    /// For best performance use script runner for a sequence of operations.
    static code run(const operation& op, Program& program);

private:
    static result run_op(const operation& op, Program& program);
};

} // namespace machine
} // namespace libbitcoin

#include <bitcoin/bitcoin/impl/machine/interpreter.ipp>

#endif
