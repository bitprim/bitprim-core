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
#include <bitcoin/bitcoin/config/parser.hpp>

#include <string>
#include <sstream>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/throw_exception.hpp>
#include <bitcoin/bitcoin/unicode/ifstream.hpp>
#include <bitcoin/bitcoin/multi_crypto_support.hpp>

#include <iostream>

namespace libbitcoin {
namespace config {

using namespace boost::filesystem;
using namespace boost::program_options;
using namespace boost::system;

// The error is obtained from boost, which circumvents our localization.
// English-only hack to patch missing arg name in boost exception message.
std::string parser::format_invalid_parameter(std::string const& message) {
    std::string clean_message(message);
    boost::replace_all(clean_message, "for option is invalid", "is invalid");
    return "Error: " + clean_message;
}

path parser::get_config_option(variables_map& variables, std::string const& name) {
    // read config from the map so we don't require an early notify
    const auto& config = variables[name];
    // prevent exception in the case where the config variable is not set
    if (config.empty())
        return path();
    return config.as<path>();
}

bool parser::get_option(variables_map& variables, std::string const& name) {
    // Read settings from the map so we don't require an early notify call.
    const auto& variable = variables[name];

    // prevent exception in the case where the settings variable is not set.
    if (variable.empty())
        return false;

    return variable.as<bool>();
}

void parser::load_command_variables(variables_map& variables, int argc, const char* argv[])
{
    const auto options = load_options();
    const auto arguments = load_arguments();
    auto command_parser = command_line_parser(argc, argv).options(options)
        /*.allow_unregistered()*/.positional(arguments);
    store(command_parser.run(), variables);
}

void parser::load_environment_variables(variables_map& variables, std::string const& prefix) {
    const auto& environment_variables = load_environment();
    const auto environment = parse_environment(environment_variables, prefix);
    store(environment, variables);
}

// int parser::load_configuration_variables(variables_map& variables, std::string const& option_name) {
load_error parser::load_configuration_variables(variables_map& variables, std::string const& option_name) {
    auto const config_path = get_config_option(variables, option_name);
    return load_configuration_variables_path(variables, config_path);
}

load_error parser::load_configuration_variables_path(variables_map& variables, boost::filesystem::path const& config_path) {
    auto const config_settings = load_settings();

    // If the existence test errors out we pretend there's no file :/.
    error_code code;
    if ( ! config_path.empty()) {
        if (exists(config_path, code)) {
            auto const& path = config_path.string();
            bc::ifstream file(path);

            if ( ! file.good()) {
                BOOST_THROW_EXCEPTION(reading_file(path.c_str()));
            }

            auto const config = parse_config_file(file, config_settings);
            store(config, variables);
            return load_error::success;
        } else {
            return load_error::non_existing_file;
        }
    }

    // Loading from an empty stream causes the defaults to populate.
    std::stringstream stream;
    auto const config = parse_config_file(stream, config_settings);
    store(config, variables);
    return load_error::default_config;
}

// static
config::checkpoint::list parser::default_checkpoints(bool easy_blocks, bool retarget) {
    config::checkpoint::list checkpoints;

    //TODO(fernando): Set Litecoin checkpoints
#if defined(BITPRIM_CURRENCY_BCH)
    if (get_network(easy_blocks, retarget) == config::settings::testnet) {
        // BCH Testnet
        checkpoints.reserve(17);
        checkpoints.emplace_back("000000000933ea01ad0ee984209779baaec3ced90fa3f408719526f8d77f4943", 0);
        checkpoints.emplace_back("00000000009e2958c15ff9290d571bf9459e93b19765c6801ddeccadbb160a1e", 100000);
        checkpoints.emplace_back("0000000000287bffd321963ef05feab753ebe274e1d78b2fd4e2bfe9ad3aa6f2", 200000);
        checkpoints.emplace_back("000000000000226f7618566e70a2b5e020e29579b46743f05348427239bf41a1", 300000);
        checkpoints.emplace_back("000000000598cbbb1e79057b79eef828c495d4fc31050e6b179c57d07d00367c", 400000);
        checkpoints.emplace_back("000000000001a7c0aaa2630fbb2c0e476aafffc60f82177375b2aaa22209f606", 500000);
        checkpoints.emplace_back("000000000000624f06c69d3a9fe8d25e0a9030569128d63ad1b704bbb3059a16", 600000);
        checkpoints.emplace_back("000000000000406178b12a4dea3b27e13b3c4fe4510994fd667d7c1e6a3f4dc1", 700000);
        checkpoints.emplace_back("0000000000209b091d6519187be7c2ee205293f25f9f503f90027e25abf8b503", 800000);
        checkpoints.emplace_back("0000000000356f8d8924556e765b7a94aaebc6b5c8685dcfa2b1ee8b41acd89b", 900000);
        checkpoints.emplace_back("0000000000478e259a3eda2fafbeeb0106626f946347955e99278fe6cc848414", 1000000);
        checkpoints.emplace_back("00000000001c2fb9880485b1f3d7b0ffa9fabdfd0cf16e29b122bb6275c73db0", 1100000);

        //2017-Aug Hardfork - Cash UAHF
        checkpoints.emplace_back("00000000f17c850672894b9a75b63a1e72830bbd5f4c8889b5c1a80e7faef138", 1155875); //time: 1501595915 - Aug 01, 2017 01:58:35 PM
        checkpoints.emplace_back("00000000000e38fef93ed9582a7df43815d5c2ba9fd37ef70c9a0ea4a285b8f5", 1155876); //time: 1501609304 - Aug 01, 2017 05:41:44 PM

        //2017-Nov Hardfork - DAA - (1510600000)
        checkpoints.emplace_back("00000000001149a812d6ecb71aea7f298fd1b29aefb773fe380c1f3649c24b84", 1188696); //time: 1510603643 - Nov 13, 2017 07:07:23 PM
        checkpoints.emplace_back("0000000000170ed0918077bde7b4d36cc4c91be69fa09211f748240dabe047fb", 1188697); //time: 1510603645 - Nov 13, 2017 07:07:25 PM

        checkpoints.emplace_back("00000000d91bdbb5394bcf457c0f0b7a7e43eb978e2d881b6c2a4c2756abc558", 1200000);

        //2018-May Hardfork - (1526400000)
        checkpoints.emplace_back("00000000001e844212a0d7db21b5cca7cb3ceca7815a4a3f6c6a9e4de4d95049", 1233077); //time: 1526412819 - May 15, 2018 4:33:39 PM
        checkpoints.emplace_back("0000000000327972b8470c11755adf8f4319796bafae01f5a6650490b98a17db", 1233078); //time: 1526414036 - May 15, 2018 4:53:56 PM

        //2018-Nov Hardfork - (1542300000)
        checkpoints.emplace_back("00000000000001fae0095cd4bea16f1ce8ab63f3f660a03c6d8171485f484b24", 1267996); //time: 1542300039 - Nov 15, 2018 2:00:05 PM
        checkpoints.emplace_back("00000000000002773f8970352e4a3368a1ce6ef91eb606b64389b36fdbf1bd56", 1267997); //time: 1542300144 - Nov 15, 2018 2:13:43 PM

        //2019-May Hardfork - (1557921600)

    } else if (get_network(easy_blocks, retarget) == config::settings::mainnet) {
        // BCH Mainnet
        checkpoints.reserve(32);
        checkpoints.emplace_back("000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f", 0);
        checkpoints.emplace_back("0000000069e244f73d78e8fd29ba2fd2ed618bd6fa2ee92559f542fdb26e7c1d", 11111);
        checkpoints.emplace_back("000000002dd5588a74784eaa7ab0507a18ad16a236e7b1ce69f00d7ddfb5d0a6", 33333);
        checkpoints.emplace_back("00000000001e1b4903550a0b96e9a9405c8a95f387162e4944e8d9fbe501cd6a", 68555);
        checkpoints.emplace_back("00000000006a49b14bcf27462068f1264c961f11fa2e0eddd2be0791e1d4124a", 70567);
        checkpoints.emplace_back("0000000000573993a3c9e41ce34471c079dcf5f52a0e824a81e7f953b8661a20", 74000);
        checkpoints.emplace_back("00000000000291ce28027faea320c8d2b054b2e0fe44a773f3eefb151d6bdc97", 105000);
        checkpoints.emplace_back("000000000000774a7f8a7a12dc906ddb9e17e75d684f15e00f8767f9e8f36553", 118000);
        checkpoints.emplace_back("00000000000005b12ffd4cd315cd34ffd4a594f430ac814c91184a0d42d2b0fe", 134444);
        checkpoints.emplace_back("000000000000033b512028abb90e1626d8b346fd0ed598ac0a3c371138dce2bd", 140700);
        checkpoints.emplace_back("000000000000099e61ea72015e79632f216fe6cb33d7899acb35b75c8303b763", 168000);
        checkpoints.emplace_back("000000000000059f452a5f7340de6682a977387c17010ff6e6c3bd83ca8b1317", 193000);
        checkpoints.emplace_back("000000000000048b95347e83192f69cf0366076336c639f9b7228e9ba171342e", 210000);
        checkpoints.emplace_back("00000000000001b4f4b433e81ee46494af945cf96014816a4e2370f11b23df4e", 216116);
        checkpoints.emplace_back("00000000000001c108384350f74090433e7fcf79a606b8e797f065b130575932", 225430);
        checkpoints.emplace_back("000000000000003887df1f29024b06fc2200b55f8af8f35453d7be294df2d214", 250000);
        checkpoints.emplace_back("0000000000000001ae8c72a0b0c301f67e3afca10e819efa9041e458e9bd7e40", 279000);
        checkpoints.emplace_back("00000000000000004d9b4ef50f0f9d686fd69db2e03af35a100370c64632a983", 295000);
        checkpoints.emplace_back("000000000000000082ccf8f1557c5d40b21edabb18d2d691cfbf87118bac7254", 300000);
        checkpoints.emplace_back("00000000000000000409695bce21828b31a7143fa35fcab64670dd337a71425d", 325000);
        checkpoints.emplace_back("0000000000000000053cf64f0400bb38e0c4b3872c38795ddde27acb40a112bb", 350000);
        checkpoints.emplace_back("000000000000000009733ff8f11fbb9575af7412df3fae97f382376709c965dc", 375000);
        checkpoints.emplace_back("000000000000000004ec466ce4732fe6f1ed1cddc2ed4b328fff5224276e3f6f", 400000);
        checkpoints.emplace_back("00000000000000000142adfebcb9a0aa75f0c4980dd5c7dd17062bf7de77c16d", 425000);
        checkpoints.emplace_back("0000000000000000014083723ed311a461c648068af8cef8a19dcd620c07a20b", 450000);
        checkpoints.emplace_back("0000000000000000017c42fd88e78ab02c5f5c684f8344e1f5c9e4cebecde71c", 475000);

        //2017-Aug Hardfork - Cash UAHF
        checkpoints.emplace_back("0000000000000000011865af4122fe3b144e2cbeea86142e8ff2fb4107352d43", 478558);   //time: 1501593374 - Aug 01, 2017 01:16:14 PM
        checkpoints.emplace_back("000000000000000000651ef99cb9fcbe0dadde1d424bd9f15ff20136191a5eec", 478559);   //time: 1501611161 - Aug 01, 2017 06:12:41 PM

        checkpoints.emplace_back("000000000000000005e14d3f9fdfb70745308706615cfa9edca4f4558332b201", 500000);

        //2017-Nov Hardfork - DAA - (1510600000)
        checkpoints.emplace_back("0000000000000000008088d63f48da98b7352ad7c4c85f3d90b657cf50ff1ede", 504030);  //time: 1510603980 - Nov 13, 2017 07:13:00 PM
        checkpoints.emplace_back("0000000000000000011ebf65b60d0a3de80b8175be709d653b4c1a1beeb6ab9c", 504031);  //time: 1510606688 - Nov 13, 2017 07:58:08 PM

        checkpoints.emplace_back("0000000000000000001b09302aa6a8dc65b7542dd195866907dd4e4ccba30d58", 515000);

        //2018-May Hardfork - (1526400000)
        checkpoints.emplace_back("00000000000000000031687b7320832e5035abe8e3f81fb71517fc541765de83", 530358);  //time: 1526408267 - May 15, 2018 3:17:47 PM
        checkpoints.emplace_back("0000000000000000011ada8bd08f46074f44a8f155396f43e38acf9501c49103", 530359);  //time: 1526410186 - May 15, 2018 3:49:46 PM

        checkpoints.emplace_back("000000000000000000fc66aae55a178fec2ba2f2bc86eb6c6f632b5bc2b40af1", 545000);  //time: 1526408267 - May 15, 2018 3:17:47 PM

        //2018-Nov Hardfork - (1542300000)
        checkpoints.emplace_back("00000000000000000102d94fde9bd0807a2cc7582fe85dd6349b73ce4e8d9322", 556766);  //time: 1542304321 - Nov 15, 2018 2:52:01 PM
        checkpoints.emplace_back("0000000000000000004626ff6e3b936941d341c5932ece4357eeccac44e6d56c", 556767);  //time: 1542304936 - Nov 15, 2018 3:02:16 PM

        //2019-May Hardfork - (1557921600)
    } else {
        // BCH Regtest
        checkpoints.reserve(1);
        checkpoints.emplace_back("0f9188f13cb7b2c71f2a335e3a4fc328bf5beb436012afca590b1a11466e2206", 0);
    }
#elif defined(BITPRIM_CURRENCY_BTC)
    if (get_network(easy_blocks, retarget) == config::settings::testnet) {
        // BTC Testnet
        checkpoints.reserve(15);
        checkpoints.emplace_back("000000000933ea01ad0ee984209779baaec3ced90fa3f408719526f8d77f4943", 0);
        checkpoints.emplace_back("00000000009e2958c15ff9290d571bf9459e93b19765c6801ddeccadbb160a1e", 100000);
        checkpoints.emplace_back("0000000000287bffd321963ef05feab753ebe274e1d78b2fd4e2bfe9ad3aa6f2", 200000);
        checkpoints.emplace_back("000000000000226f7618566e70a2b5e020e29579b46743f05348427239bf41a1", 300000);
        checkpoints.emplace_back("000000000598cbbb1e79057b79eef828c495d4fc31050e6b179c57d07d00367c", 400000);
        checkpoints.emplace_back("000000000001a7c0aaa2630fbb2c0e476aafffc60f82177375b2aaa22209f606", 500000);
        checkpoints.emplace_back("000000000000624f06c69d3a9fe8d25e0a9030569128d63ad1b704bbb3059a16", 600000);
        checkpoints.emplace_back("000000000000406178b12a4dea3b27e13b3c4fe4510994fd667d7c1e6a3f4dc1", 700000);
        checkpoints.emplace_back("0000000000209b091d6519187be7c2ee205293f25f9f503f90027e25abf8b503", 800000);
        checkpoints.emplace_back("0000000000356f8d8924556e765b7a94aaebc6b5c8685dcfa2b1ee8b41acd89b", 900000);
        checkpoints.emplace_back("0000000000478e259a3eda2fafbeeb0106626f946347955e99278fe6cc848414", 1000000);
        checkpoints.emplace_back("00000000001c2fb9880485b1f3d7b0ffa9fabdfd0cf16e29b122bb6275c73db0", 1100000);

        //2017-Aug Hardfork - Cash UAHF     
        checkpoints.emplace_back("00000000f17c850672894b9a75b63a1e72830bbd5f4c8889b5c1a80e7faef138", 1155875);
        checkpoints.emplace_back("0000000093b3cdf2b50a05fa1527810f52d6826781916ef129098e06ee03fb18", 1155876);

        checkpoints.emplace_back("00000000000025c23a19cc91ad8d3e33c2630ce1df594e1ae0bf0eabe30a9176", 1200000);
        checkpoints.emplace_back("000000007ec390190c60b5010a8ea14f5ce53e35be684eacc36486fec3b34744", 1300000);
        checkpoints.emplace_back("000000000000fce208da3e3b8afcc369835926caa44044e9c2f0caa48c8eba0f", 1400000);
    } else if (get_network(easy_blocks, retarget) == config::settings::mainnet) {
        // BTC Mainnet
        checkpoints.reserve(30);
        checkpoints.emplace_back("000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f", 0);
        checkpoints.emplace_back("0000000069e244f73d78e8fd29ba2fd2ed618bd6fa2ee92559f542fdb26e7c1d", 11111);
        checkpoints.emplace_back("000000002dd5588a74784eaa7ab0507a18ad16a236e7b1ce69f00d7ddfb5d0a6", 33333);
        checkpoints.emplace_back("00000000001e1b4903550a0b96e9a9405c8a95f387162e4944e8d9fbe501cd6a", 68555);
        checkpoints.emplace_back("00000000006a49b14bcf27462068f1264c961f11fa2e0eddd2be0791e1d4124a", 70567);
        checkpoints.emplace_back("0000000000573993a3c9e41ce34471c079dcf5f52a0e824a81e7f953b8661a20", 74000);
        checkpoints.emplace_back("00000000000291ce28027faea320c8d2b054b2e0fe44a773f3eefb151d6bdc97", 105000);
        checkpoints.emplace_back("000000000000774a7f8a7a12dc906ddb9e17e75d684f15e00f8767f9e8f36553", 118000);
        checkpoints.emplace_back("00000000000005b12ffd4cd315cd34ffd4a594f430ac814c91184a0d42d2b0fe", 134444);
        checkpoints.emplace_back("000000000000033b512028abb90e1626d8b346fd0ed598ac0a3c371138dce2bd", 140700);
        checkpoints.emplace_back("000000000000099e61ea72015e79632f216fe6cb33d7899acb35b75c8303b763", 168000);
        checkpoints.emplace_back("000000000000059f452a5f7340de6682a977387c17010ff6e6c3bd83ca8b1317", 193000);
        checkpoints.emplace_back("000000000000048b95347e83192f69cf0366076336c639f9b7228e9ba171342e", 210000);
        checkpoints.emplace_back("00000000000001b4f4b433e81ee46494af945cf96014816a4e2370f11b23df4e", 216116);
        checkpoints.emplace_back("00000000000001c108384350f74090433e7fcf79a606b8e797f065b130575932", 225430);
        checkpoints.emplace_back("000000000000003887df1f29024b06fc2200b55f8af8f35453d7be294df2d214", 250000);
        checkpoints.emplace_back("0000000000000001ae8c72a0b0c301f67e3afca10e819efa9041e458e9bd7e40", 279000);
        checkpoints.emplace_back("00000000000000004d9b4ef50f0f9d686fd69db2e03af35a100370c64632a983", 295000);
        checkpoints.emplace_back("000000000000000082ccf8f1557c5d40b21edabb18d2d691cfbf87118bac7254", 300000);
        checkpoints.emplace_back("00000000000000000409695bce21828b31a7143fa35fcab64670dd337a71425d", 325000);
        checkpoints.emplace_back("0000000000000000053cf64f0400bb38e0c4b3872c38795ddde27acb40a112bb", 350000);
        checkpoints.emplace_back("000000000000000009733ff8f11fbb9575af7412df3fae97f382376709c965dc", 375000);
        checkpoints.emplace_back("000000000000000004ec466ce4732fe6f1ed1cddc2ed4b328fff5224276e3f6f", 400000);
        checkpoints.emplace_back("00000000000000000142adfebcb9a0aa75f0c4980dd5c7dd17062bf7de77c16d", 425000);
        checkpoints.emplace_back("0000000000000000014083723ed311a461c648068af8cef8a19dcd620c07a20b", 450000);
        checkpoints.emplace_back("0000000000000000017c42fd88e78ab02c5f5c684f8344e1f5c9e4cebecde71c", 475000);

        //2017-Aug Hardfork - Cash UAHF
        checkpoints.emplace_back("0000000000000000011865af4122fe3b144e2cbeea86142e8ff2fb4107352d43", 478558);
        checkpoints.emplace_back("00000000000000000019f112ec0a9982926f1258cdcc558dd7c3b7e5dc7fa148", 478559);

        checkpoints.emplace_back("00000000000000000024fb37364cbf81fd49cc2d51c09c75c35433c3a1945d04", 500000);
        checkpoints.emplace_back("0000000000000000003ca88d20895d2535f304cca8afb08e7e5503fcac1da752", 515000);
        checkpoints.emplace_back("000000000000000000024e9be1c7b56cab6428f07920f21ad8457221a91371ae", 530000);
    } else {
        // BCH Regtest
        checkpoints.reserve(1);
        checkpoints.emplace_back("0f9188f13cb7b2c71f2a335e3a4fc328bf5beb436012afca590b1a11466e2206", 0);    //TODO(fernando): check if BTC-Regtest-genesis == BCH-Regtest-genesis
    }
#endif //defined(BITPRIM_CURRENCY_BCH)

    return checkpoints;
}

// static
void parser::fix_checkpoints(bool easy_blocks, bool retarget, config::checkpoint::list& checkpoints) {
    auto const def_checkpoints = default_checkpoints(easy_blocks, retarget);

    auto const it = std::max_element(def_checkpoints.begin(), def_checkpoints.end(), [](checkpoint const& x, checkpoint const& y) {
        return x.height() < y.height();
    });

    if (it != def_checkpoints.end()) {
        auto const max_checkpoint_height = it->height();

        checkpoints.erase(std::remove_if(checkpoints.begin(), checkpoints.end(), [max_checkpoint_height](checkpoint const& x) {
            return x.height() <= max_checkpoint_height;
        }), checkpoints.end());

        checkpoints.insert(checkpoints.begin(), def_checkpoints.begin(), def_checkpoints.end());
    }
}    


} // namespace config
} // namespace libbitcoin
