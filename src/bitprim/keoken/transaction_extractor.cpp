/**
 * Copyright (c) 2017-2018 Bitprim Inc.
 *
 * This file is part of Bitprim.
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

#include <bitprim/keoken/transaction_extractor.hpp>

#include <type_traits>

#include <bitcoin/bitcoin/machine/opcode.hpp>
#include <bitcoin/bitcoin/machine/operation.hpp>

#include <bitprim/keoken/constants.hpp>

namespace bitprim {
namespace keoken {

using libbitcoin::data_chunk;
using libbitcoin::machine::opcode;
using libbitcoin::machine::operation;
 
template <typename I>
data_chunk get_keoken_data(I f, I l) {
    //precondition:  
    //postcondition: if return value is an empty vector, it is not a keoken output.

    if (f == l) return data_chunk{};

    if (f->code() != opcode::return_) return data_chunk{};

    ++f;        //me muevo al siguiente operation (de Eric)
    if (f == l) return data_chunk{};

    if (f->code() != opcode::push_size_4) return data_chunk{};

    if ( ! std::equal(f->data().begin(), f->data().end(), protocol_name)) {
        return data_chunk{};
    }

    ++f;
    if (f == l) return data_chunk{};

    // if (f->code() != opcode::push_size_4) return data_chunk{};

    if (to_underlying(f->code()) < kp_min_size) return data_chunk{};
    if (to_underlying(f->code()) > kp_max_size) return data_chunk{};

    return f->data();   //TODO(fernando): check if we can std::move this vector
}

data_chunk get_keoken_data(operation::list const& ops) {
    return get_keoken_data(ops.begin(), ops.end());
}

data_chunk first_keoken_output(libbitcoin::chain::transaction const& tx) {
    //precondition: tx is a confirmed transaction
    for (auto const& out : tx.outputs()) {
        auto const& keo_data = get_keoken_data(out.script().operations());
        if ( ! keo_data.empty()) {
            return keo_data;
        }
    }
    return data_chunk{};
}


/*
raw = "6a296f6d6e69000000320100010000000000005265616c57616e6461506574650000000000000000000064";
OP_RETURN=6a
SIZE = 29                             (41 dec = bytes a leer)
PREFIJO=6f6d6e69                      (omni)
TX-VERSION=0000                       (0)
TX-TYPE=0032                          (50 dec)
ECOSYSTEM=01                          (1) (main)
PROPERTY-TYPE=0001                    (1) (indivisible)
PREV-PROPERTY-ID=00000000             (0)
CATEGORY=00                           (\0)
SUBCATEGORY=00                        (\0)
NAME=5265616c57616e64615065746500     (RealWandaPete\0)
URL=00                                (\0)
DATA=00                               (\0)
COINS=0000000000000064                (100 dec)
*/

// if raw[:16] == "6a066b656f6b656e":



// https://www.blocktrail.com/BTC/tx/6487e1ef8817af1999b9c597dea420e4fe13340cbb9dfa8dc37b82c7d6ea6e94#tx_messages
// https://blockchain.info/tx/6487e1ef8817af1999b9c597dea420e4fe13340cbb9dfa8dc37b82c7d6ea6e94?format=hex
// 0100000001db10e80cb3a10156f3bdbba045d93bd2dad5e9bc483c5d012728fcedb0f64708000000008a47304402203f701d9dbe0c313fbccd5c91d74272689c237b7eaffb1f70ce83f3f04b03346802200ca39f9e5892b45c9175d18d1216f334517219d8b2822f519c2b81de9d608196014104f8f2e7a54778b247b2900b83dda20cb3a82b936865b8dbec1b980c0f50a6699d4f315e288054d28d20e933be410308e2b6e4641c02b452efc4a7f2a1f7b68be0ffffffff03c5114a00000000001976a91451400a3894126949eb493cc8ea73c11615c2787688ac22020000000000001976a914e5710c7722dbc33ae8eb8074d5c87d33eaada45188ac0000000000000000166a146f6d6e690000000000000173000000000000000100000000

// Decoded
/*

{
    "addresses": [
        "1MvBAh2L4vcGL7EqguJhp65wUXCEvJAo8N", 
        "18QcT7Q6zFwbFecLy7CfrbNCWH3hkDCB8K"
    ], 
    "block_height": -1, 
    "block_index": -1, 
    "confirmations": 0, 
    "data_protocol": "omni", 
    "double_spend": false, 
    "fees": 3270, 
    "hash": "6487e1ef8817af1999b9c597dea420e4fe13340cbb9dfa8dc37b82c7d6ea6e94", 
    "inputs": [
        {
            "addresses": [
                "18QcT7Q6zFwbFecLy7CfrbNCWH3hkDCB8K"
            ], 
            "age": 532507, 
            "output_index": 0, 
            "output_value": 4858029, 
            "prev_hash": "0847f6b0edfc2827015d3c48bce9d5dad23bd945a0bbbdf35601a1b30ce810db", 
            "script": "47304402203f701d9dbe0c313fbccd5c91d74272689c237b7eaffb1f70ce83f3f04b03346802200ca39f9e5892b45c9175d18d1216f334517219d8b2822f519c2b81de9d608196014104f8f2e7a54778b247b2900b83dda20cb3a82b936865b8dbec1b980c0f50a6699d4f315e288054d28d20e933be410308e2b6e4641c02b452efc4a7f2a1f7b68be0", 
            "script_type": "pay-to-pubkey-hash", 
            "sequence": 4294967295
        }
    ], 
    "outputs": [
        {
            "addresses": [
                "18QcT7Q6zFwbFecLy7CfrbNCWH3hkDCB8K"
            ], 
            "script": "76a91451400a3894126949eb493cc8ea73c11615c2787688ac", 
            "script_type": "pay-to-pubkey-hash", 
            "value": 4854213
        }, 
        {
            "addresses": [
                "1MvBAh2L4vcGL7EqguJhp65wUXCEvJAo8N"
            ], 
            "script": "76a914e5710c7722dbc33ae8eb8074d5c87d33eaada45188ac", 
            "script_type": "pay-to-pubkey-hash", 
            "value": 546
        }, 
        {
            "addresses": null, 
            "data_hex": "6f6d6e6900000000000001730000000000000001", 
            "script": "6a146f6d6e6900000000000001730000000000000001", 
            "script_type": "null-data", 
            "value": 0
        }
    ], 
    "preference": "medium", 
    "received": "2018-07-20T16:32:11.817290219Z", 
    "relayed_by": "54.226.47.150", 
    "size": 288, 
    "total": 4854759, 
    "ver": 1, 
    "vin_sz": 1, 
    "vout_sz": 3
}

*/



/*
Formatos:
    TX Asset Creation (completa a nivel BCH):
        input 0: el wallet de este input es el owner de los tokens creados
                 paga guita de los fees   
        input 1: ignorada a nivel keoken, paga guita de los fees u otra cosa
        ...
        input n: ignorada a nivel keoken, paga guita de los fees u otra cosa
        ----------------------------------------------------------------------
        output 0: ignorada a nivel Keoken
        output 1: ignorada a nivel Keoken
        ...
id:1    output j: OP_RETURN 00KP version:0, type:0, name: Piticoin, amount: 100
                  Este output es el que se procesa a nivel Keoken
        ...
        output n: OP_RETURN 00KP version:0, type:0, name: Wandacoin, amount: 200
                  Ignorada
        ...

    TX Sends Token (completa a nivel BCH):
        input 0: tiene guita y manda (la wallet de prev out tiene saldo disponible en el asset)
                 Unlocking script (para desbloquear la guita de los fees)
                 Tiene BCH
                 Tiene Saldo del Asset
        input 1: Puede haber un segundo input, para intencambiar BCH...
        input 2: Puede haber un segundo input Keoken????
        ----------------------------------------------------------------------
        output 0 -> target: wallet que recibe los tokens, bch amount: dust
        output 1 -> puede ser vuelto, solamente consideradas a nivel BCH
        output 2 -> OP_RETURN 00KP version:0, type:1, id: 1, amount: 10
        output 3 -> puede ser vuelto, solamente consideradas a nivel BCH







    TX BCH Previa:
        input 0: - ???
        ----------------------------------------------------------------------
        output 0 -> amount, Locking Script #1

    TX BCH Normal:
        input 0: - previous output: txid: hash, index: nro del output dentro de la txid
                 - unlocking script (script que desbloquea el locking script que está en el prevout)
                 - Unlocking Script #1 que matchea con Locking Script #1
        ----------------------------------------------------------------------
        output 0 -> ???



Ejemplos:

    TX 0, Asset Creation, Non-Standard BCH Transaction
        input 0: el wallet de este input es el owner de los tokens creados
                 paga guita de los fees   
        input 1: ignorada a nivel keoken, paga guita de los fees u otra cosa
        input n: ignorada a nivel keoken, paga guita de los fees u otra cosa
        ----------------------------------------------------------------------
        output 0: ignorada a nivel Keoken
        output 1: ignorada a nivel Keoken
id:n    output j: OP_RETURN 00KP version:0, type:0, name: Piticoin, amount: 100
                  Este output es el que se procesa a nivel Keoken
        output n: OP_RETURN 00KP version:0, type:0, name: Wandacoin, amount: 200
                  Ignorada, TX non-standard, Keoken rule.
                  Keoken procesa transacciones standard y no-standard, pero en el caso
                  de que la TX sea no-estandar sólo vamos a procesar el primer 
                  Keoken-output (OP-RETURN 00KP)
        ...

    TX 1, Asset Creation, Standard BCH Transaction
        input 0: el wallet de este input es el owner de los tokens creados
                 paga guita de los fees   
        input 1: ignorada a nivel keoken, paga guita de los fees u otra cosa
        ...
        input n: ignorada a nivel keoken, paga guita de los fees u otra cosa
        ----------------------------------------------------------------------
        output 0: ignorada a nivel Keoken
        output 1: ignorada a nivel Keoken
        ...
id:n+1  output n: OP_RETURN 00KP version:0, type:0, name: Wandacoin, amount: 200
        ...

    TX 2, Invalid Keoken, Standard BCH Transaction
        input 0: el wallet de este input es el owner de los tokens creados
                 paga guita de los fees   
        input 1: ignorada a nivel keoken, paga guita de los fees u otra cosa
        ...
        input n: ignorada a nivel keoken, paga guita de los fees u otra cosa
        ----------------------------------------------------------------------
        output 0: ignorada a nivel Keoken
        output 1: ignorada a nivel Keoken
        ...
        output n: OP_RETURN 00KP FRUTA
                  Ignorada
        ...

    TX 3, Invalid Keoken, Standard BCH Transaction
        input 0: el wallet de este input es el owner de los tokens creados
                 paga guita de los fees   
        input 1: ignorada a nivel keoken, paga guita de los fees u otra cosa
        ...
        input n: ignorada a nivel keoken, paga guita de los fees u otra cosa
        ----------------------------------------------------------------------
        output 0: ignorada a nivel Keoken
        output 1: ignorada a nivel Keoken
        ...
        output n: OP_RETURN 00KP version:0, type:0 ... falta el resto
                  Ignorada
        ...

    TX 4, Asset Creation, Standard BCH Transaction
        input 0: el wallet de este input es el owner de los tokens creados
                 paga guita de los fees   
        input 1: ignorada a nivel keoken, paga guita de los fees u otra cosa
        ...
        input n: ignorada a nivel keoken, paga guita de los fees u otra cosa
        ----------------------------------------------------------------------
        output 0: ignorada a nivel Keoken
        output 1: ignorada a nivel Keoken
        ...
id:n+2  output n: OP_RETURN 00KP version:0, type:0, name: EhAmigo, amount: 300


*/



/*
API:

    - crear un asset                           (Guille y Rama)
        Input:

        Acciones:
        - crear un objeto keoken interno
        - crea la TX con
        - envia la TX a la red (organize)

    - send tokens                               (Guille y Rama)
        Input:
        Acciones:
        - crear un objeto keoken interno
        - crea la TX con
        - envia la TX a la red (organize)

    - listar todos los assets de la blockchain 
        - nombre de asset
        - cantidad (que es la que se creó)
        - id (generado con el conteo)
        - dueno
        - block height donde se creó
        - Tx donde se creó

    - balance en token para un address (de todos los assets del address)
        - nombre de asset
        - cantidad (que es la que se creó)
        - id (generado con el conteo)
        - balance en tokens

    - cuenta corriente (balance con detalle) en tokens para un address (de todos los assets del address)
        - nombre de asset
        - cantidad (que es la que se creó)
        - id (generado con el conteo)
        - detalle de TXs que mueven

    - 

*/    

} // namespace keoken
} // namespace bitprim
