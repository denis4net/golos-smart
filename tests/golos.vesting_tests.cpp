#include "golos_tester.hpp"
#include "contracts.hpp"
#include <Runtime/Runtime.h>

#include <fc/variant_object.hpp>

using namespace eosio::testing;
using namespace eosio;
using namespace eosio::chain;
using namespace eosio::testing;
using namespace fc;
using namespace std;

using mvo = fc::mutable_variant_object;

class golos_vesting_tester : public golos_tester {
public:

    golos_vesting_tester(): golos_tester() {
        produce_blocks(2);

        create_accounts( { N(sania), N(pasha), N(tania), N(golos.vest), N(golos.ctrl), N(eosio.token), N(golos.emiss), N(golos.issuer) } );
        produce_blocks(2);

        install_contract(N(eosio.token), contracts::token_wasm(), contracts::token_abi(), abi_ser_t);
        install_contract(N(golos.vest), contracts::vesting_wasm(), contracts::vesting_abi(), abi_ser_v);
        abi_serializer t;
        install_contract(N(golos.ctrl), contracts::ctrl_wasm(), contracts::ctrl_abi(), t);
    }

    action_result push_action( const account_name& signer, const action_name &name, const variant_object &data ) {
        string action_type_name = abi_ser_t.get_action_type(name);

        action act;
        act.account = N(eosio.token);
        act.name    = name;
        act.data    = abi_ser_t.variant_to_binary( action_type_name, data, abi_serializer_max_time );

        return base_tester::push_action( std::move(act), uint64_t(signer));
    }

    action_result push_action_golos_vesting( const account_name& signer, const action_name &name,
                                             const variant_object &data ) {
        string action_type_name = abi_ser_v.get_action_type(name);

        action act;
        act.account = N(golos.vest);
        act.name    = name;
        act.data    = abi_ser_v.variant_to_binary( action_type_name, data, abi_serializer_max_time );

        return base_tester::push_action( std::move(act), uint64_t(signer));
    }

    fc::variant get_stats( const string& symbolname )
    {
        auto symb = eosio::chain::symbol::from_string(symbolname);
        auto symbol_code = symb.to_symbol_code().value;
        vector<char> data = get_row_by_account( N(eosio.token), symbol_code, N(stat), symbol_code );
        return data.empty() ? fc::variant() : abi_ser_t.binary_to_variant( "currency_stats", data, abi_serializer_max_time );
    }

    fc::variant get_account( account_name acc, const string& symbolname) {
        auto symb = eosio::chain::symbol::from_string(symbolname);
        auto symbol_code = symb.to_symbol_code().value;
        vector<char> data = get_row_by_account( N(eosio.token), acc, N(accounts), symbol_code );
        return data.empty() ? fc::variant() : abi_ser_t.binary_to_variant( "account", data, abi_serializer_max_time );
    }

    fc::variant get_account_vesting( account_name acc, const string& symbolname) {
        auto symb = eosio::chain::symbol::from_string(symbolname);
        auto symbol_code = symb.to_symbol_code().value;
        vector<char> data = get_row_by_account( N(golos.vest), acc, N(balances), symbol_code );
        return data.empty() ? fc::variant() : abi_ser_v.binary_to_variant( "user_balance", data, abi_serializer_max_time );
    }

    action_result create( account_name issuer, asset maximum_supply ) {

        return push_action( N(eosio.token), N(create), mvo()
                            ( "issuer", issuer)
                            ( "maximum_supply", maximum_supply)
                            );
    }

    action_result issue( account_name issuer, account_name to, asset quantity, string memo ) {
        return push_action( issuer, N(issue), mvo()
                            ( "to", to)
                            ( "quantity", quantity)
                            ( "memo", memo)
                            );
    }

    action_result transfer( account_name from, account_name to, asset quantity, string memo ) {
        return push_action( from, N(transfer), mvo()
                            ( "from", from)
                            ( "to", to)
                            ( "quantity", quantity)
                            ( "memo", memo)
                            );
    }

    action_result open_balance(account_name owner, symbol s, account_name ram_payer) {
        return push_action_golos_vesting( ram_payer, N(open), mvo()
                                          ( "owner", owner)
                                          ( "symbol", s)
                                          ( "ram_payer", ram_payer)
                                          );
    }

    action_result convert_vesting(account_name sender, account_name recipient, asset quantity) {
        return push_action_golos_vesting( sender, N(convertvg), mvo()
                                          ( "sender", sender)
                                          ( "recipient", recipient)
                                          ( "quantity", quantity)
                                          );
    }

    action_result cancel_convert_vesting(account_name sender, asset type) {
        return push_action_golos_vesting( sender, N(cancelvg), mvo()
                                          ( "sender", sender)
                                          ( "type", type)
                                          );
    }

    action_result delegate_vesting(account_name sender, account_name recipient, asset quantity, uint16_t interest_rate, uint8_t payout_strategy) {
        return push_action_golos_vesting( sender, N(delegatevg), mvo()
                                          ( "sender", sender)
                                          ( "recipient", recipient)
                                          ( "quantity", quantity)
                                          ( "interest_rate", interest_rate)
                                          ( "payout_strategy", payout_strategy)
                                          );
    }

    action_result undelegate_vesting(account_name sender, account_name recipient, asset quantity) {
        return push_action_golos_vesting( sender, N(undelegatevg), mvo()
                                          ( "sender", sender)
                                          ( "recipient", recipient)
                                          ( "quantity", quantity)
                                          );
    }

    action_result create_vesting_token(account_name creator, symbol vesting_symbol, std::vector<account_name> issuers = {}) {
        return push_action_golos_vesting( creator, N(createvest), mvo()
                                         ("creator", creator)
                                         ("symbol", vesting_symbol)
                                         ("issuers", issuers)
                                         );
    }

    action_result start_timer_trx() {
        return push_action_golos_vesting( N(golos.vest), N(timeout), mvo()("hash", 1));
    }

    void prepare_balances() {
        BOOST_REQUIRE_EQUAL( success(), create(N(golos.issuer), asset::from_string("100000.0000 GOLOS")) );
        BOOST_REQUIRE_EQUAL( success(), issue(N(golos.issuer), N(sania), asset::from_string("500.0000 GOLOS"), "issue tokens sania") );
        BOOST_REQUIRE_EQUAL( success(), issue(N(golos.issuer), N(pasha), asset::from_string("500.0000 GOLOS"), "issue tokens pasha") );
        produce_blocks(1);

        BOOST_REQUIRE_EQUAL( success(), open_balance(N(sania), symbol(4,"GOLOS"), N(sania)) );
        BOOST_REQUIRE_EQUAL( success(), open_balance(N(pasha), symbol(4,"GOLOS"), N(pasha)) );
        produce_blocks(1);

        BOOST_REQUIRE_EQUAL( success(), create_vesting_token(N(golos.issuer), symbol(4,"GOLOS")) );
        BOOST_REQUIRE_EQUAL( success(), transfer(N(sania), N(golos.vest), asset::from_string("100.0000 GOLOS"), "convert token to vesting") );
        BOOST_REQUIRE_EQUAL( success(), transfer(N(pasha), N(golos.vest), asset::from_string("100.0000 GOLOS"), "convert token to vesting") );
        produce_blocks(1);

        auto sania_token_balance = get_account(N(sania), "4,GOLOS");
        REQUIRE_MATCHING_OBJECT( sania_token_balance, mvo()
                                 ("balance", "400.0000 GOLOS") );

        auto pasha_token_balance = get_account(N(pasha), "4,GOLOS");
        REQUIRE_MATCHING_OBJECT( pasha_token_balance, mvo()
                                 ("balance", "400.0000 GOLOS") );

        auto sania_vesting_balance = get_account_vesting(N(sania), "4,GOLOS");
        REQUIRE_MATCHING_OBJECT( sania_vesting_balance, mvo()
                                 ("vesting", "100.0000 GOLOS")
                                 ("delegate_vesting", "0.0000 GOLOS")
                                 ("received_vesting", "0.0000 GOLOS")
                                 ("unlocked_limit", "0.0000 GOLOS")
                                 );

        auto pasha_vesting_balance = get_account_vesting(N(pasha), "4,GOLOS");
        REQUIRE_MATCHING_OBJECT( pasha_vesting_balance, mvo()
                                 ("vesting", "100.0000 GOLOS")
                                 ("delegate_vesting", "0.0000 GOLOS")
                                 ("received_vesting", "0.0000 GOLOS")
                                 ("unlocked_limit", "0.0000 GOLOS")
                                 );
    }

    abi_serializer abi_ser_v;
    abi_serializer abi_ser_t;
};

BOOST_AUTO_TEST_SUITE(golos_vesting_tests)

BOOST_FIXTURE_TEST_CASE( test_create_tokens, golos_vesting_tester ) try {
    BOOST_REQUIRE_EQUAL( success(), create(N(eosio.token), asset::from_string("100000.0000 GOLOS")) );

    auto stats = get_stats("4,GOLOS");
    REQUIRE_MATCHING_OBJECT( stats, mvo()
                             ("supply", "0.0000 GOLOS")
                             ("max_supply", "100000.0000 GOLOS")
                             ("issuer", "eosio.token")
                             );
} FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE( test_create_vesting_for_nonexistent_token, golos_vesting_tester ) try {
    BOOST_REQUIRE_EQUAL( "assertion failure with message: unable to find key", create_vesting_token(N(golos.issuer), symbol(4,"GOLOS")));
} FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE( test_create_vesting_by_not_issuer, golos_vesting_tester ) try {
    BOOST_REQUIRE_EQUAL( success(), create(N(eosio.token), asset::from_string("100000.0000 GOLOS")) );
    BOOST_REQUIRE_EQUAL( "assertion failure with message: Only token issuer can create it", create_vesting_token(N(golos.issuer), symbol(4,"GOLOS")));
} FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE( test_create_vesting, golos_vesting_tester ) try {
    BOOST_REQUIRE_EQUAL( success(), create(N(golos.issuer), asset::from_string("100000.0000 GOLOS")) );
    BOOST_REQUIRE_EQUAL( success(), create_vesting_token(N(golos.issuer), symbol(4,"GOLOS")));
} FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE( test_issue_tokens_accounts, golos_vesting_tester ) try {
    BOOST_REQUIRE_EQUAL( success(), create(N(golos.issuer), asset::from_string("100000.0000 GOLOS")) );

    BOOST_REQUIRE_EQUAL( success(), issue(N(golos.issuer), N(sania), asset::from_string("500.0000 GOLOS"), "issue tokens sania") );
    BOOST_REQUIRE_EQUAL( success(), issue(N(golos.issuer), N(pasha), asset::from_string("500.0000 GOLOS"), "issue tokens pasha") );
    produce_blocks(1);

    auto stats = get_stats("4,GOLOS");
    REQUIRE_MATCHING_OBJECT( stats, mvo()
                             ("supply", "1000.0000 GOLOS")
                             ("max_supply", "100000.0000 GOLOS")
                             ("issuer", "golos.issuer")
                             );

    auto sania_balance = get_account(N(sania), "4,GOLOS");
    REQUIRE_MATCHING_OBJECT( sania_balance, mvo()
                             ("balance", "500.0000 GOLOS")
                             );


    auto pasha_balance = get_account(N(pasha), "4,GOLOS");
    REQUIRE_MATCHING_OBJECT( pasha_balance, mvo()
                             ("balance", "500.0000 GOLOS")
                             );
} FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE( test_convert_token_to_vesting, golos_vesting_tester ) try {
    BOOST_REQUIRE_EQUAL( success(), create(N(golos.issuer), asset::from_string("100000.0000 GOLOS")) );

    BOOST_REQUIRE_EQUAL( success(), issue(N(golos.issuer), N(sania), asset::from_string("500.0000 GOLOS"), "issue tokens sania") );
    BOOST_REQUIRE_EQUAL( success(), issue(N(golos.issuer), N(pasha), asset::from_string("500.0000 GOLOS"), "issue tokens pasha") );
    produce_blocks(1);

    BOOST_REQUIRE_EQUAL( success(), open_balance(N(sania), symbol(4,"GOLOS"), N(sania)) );
    BOOST_REQUIRE_EQUAL( success(), open_balance(N(pasha), symbol(4,"GOLOS"), N(pasha)) );
    produce_blocks(1);

    BOOST_REQUIRE_EQUAL( success(), create_vesting_token(N(golos.issuer), symbol(4,"GOLOS")) );
    BOOST_REQUIRE_EQUAL( success(), transfer(N(sania), N(golos.vest), asset::from_string("100.0000 GOLOS"), "convert token to vesting") );
    BOOST_REQUIRE_EQUAL( success(), transfer(N(pasha), N(golos.vest), asset::from_string("100.0000 GOLOS"), "convert token to vesting") );
    produce_blocks(1);

    auto sania_token_balance = get_account(N(sania), "4,GOLOS");
    REQUIRE_MATCHING_OBJECT( sania_token_balance, mvo()
                             ("balance", "400.0000 GOLOS") );

    auto pasha_token_balance = get_account(N(pasha), "4,GOLOS");
    REQUIRE_MATCHING_OBJECT( pasha_token_balance, mvo()
                             ("balance", "400.0000 GOLOS") );

    auto sania_vesting_balance = get_account_vesting(N(sania), "4,GOLOS");
    REQUIRE_MATCHING_OBJECT( sania_vesting_balance, mvo()
                             ("vesting", "100.0000 GOLOS")
                             ("delegate_vesting", "0.0000 GOLOS")
                             ("received_vesting", "0.0000 GOLOS")
                             ("unlocked_limit", "0.0000 GOLOS")
                             );

    auto pasha_vesting_balance = get_account_vesting(N(pasha), "4,GOLOS");
    REQUIRE_MATCHING_OBJECT( pasha_vesting_balance, mvo()
                             ("vesting", "100.0000 GOLOS")
                             ("delegate_vesting", "0.0000 GOLOS")
                             ("received_vesting", "0.0000 GOLOS")
                             ("unlocked_limit", "0.0000 GOLOS")
                             );
} FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE( test_convert_vesting_to_token, golos_vesting_tester ) try {
    BOOST_REQUIRE_EQUAL( success(), create(N(golos.issuer), asset::from_string("100000.0000 GOLOS")) );
    BOOST_REQUIRE_EQUAL( success(), issue(N(golos.issuer), N(sania), asset::from_string("500.0000 GOLOS"), "issue tokens sania") );
    BOOST_REQUIRE_EQUAL( success(), issue(N(golos.issuer), N(pasha), asset::from_string("500.0000 GOLOS"), "issue tokens pasha") );
    produce_blocks(1);

    BOOST_REQUIRE_EQUAL( success(), open_balance(N(sania), symbol(4,"GOLOS"), N(sania)) );
    BOOST_REQUIRE_EQUAL( success(), open_balance(N(pasha), symbol(4,"GOLOS"), N(pasha)) );
    produce_blocks(1);

    BOOST_REQUIRE_EQUAL( success(), create_vesting_token(N(golos.issuer), symbol(4,"GOLOS")) );
    BOOST_REQUIRE_EQUAL( success(), transfer(N(sania), N(golos.vest), asset::from_string("100.0000 GOLOS"), "convert token to vesting") );
    BOOST_REQUIRE_EQUAL( success(), transfer(N(pasha), N(golos.vest), asset::from_string("100.0000 GOLOS"), "convert token to vesting") );
    produce_blocks(1);

    auto sania_token_balance = get_account(N(sania), "4,GOLOS");
    REQUIRE_MATCHING_OBJECT( sania_token_balance, mvo()
                             ("balance", "400.0000 GOLOS") );

    auto pasha_token_balance = get_account(N(pasha), "4,GOLOS");
    REQUIRE_MATCHING_OBJECT( pasha_token_balance, mvo()
                             ("balance", "400.0000 GOLOS") );

    auto sania_vesting_balance = get_account_vesting(N(sania), "4,GOLOS");
    REQUIRE_MATCHING_OBJECT( sania_vesting_balance, mvo()
                             ("vesting", "100.0000 GOLOS")
                             ("delegate_vesting", "0.0000 GOLOS")
                             ("received_vesting", "0.0000 GOLOS")
                             ("unlocked_limit", "0.0000 GOLOS")
                             );

    auto pasha_vesting_balance = get_account_vesting(N(pasha), "4,GOLOS");
    REQUIRE_MATCHING_OBJECT( pasha_vesting_balance, mvo()
                             ("vesting", "100.0000 GOLOS")
                             ("delegate_vesting", "0.0000 GOLOS")
                             ("received_vesting", "0.0000 GOLOS")
                             ("unlocked_limit", "0.0000 GOLOS")
                             );


    BOOST_REQUIRE_EQUAL( success(), convert_vesting(N(sania), N(sania), asset::from_string("13.0000 GOLOS")) );
    BOOST_REQUIRE_EQUAL( success(), start_timer_trx() );
    auto delegated_auth = authority( 1, {}, {
                                         { .permission = {N(golos.vest), config::eosio_code_name}, .weight = 1}
                                     });
    set_authority( N(golos.vest), config::active_name, delegated_auth );
    produce_blocks(31);

    sania_vesting_balance = get_account_vesting(N(sania), "4,GOLOS");
    REQUIRE_MATCHING_OBJECT( sania_vesting_balance, mvo()
                             ("vesting", "99.0000 GOLOS")
                             ("delegate_vesting", "0.0000 GOLOS")
                             ("received_vesting", "0.0000 GOLOS")
                             ("unlocked_limit", "0.0000 GOLOS")
                             );

    sania_token_balance = get_account(N(sania), "4,GOLOS");
    REQUIRE_MATCHING_OBJECT( sania_token_balance, mvo()
                             ("balance", "401.0000 GOLOS") );

    produce_blocks(270);
    sania_vesting_balance = get_account_vesting(N(sania), "4,GOLOS");
    REQUIRE_MATCHING_OBJECT( sania_vesting_balance, mvo()
                             ("vesting", "90.0000 GOLOS")
                             ("delegate_vesting", "0.0000 GOLOS")
                             ("received_vesting", "0.0000 GOLOS")
                             ("unlocked_limit", "0.0000 GOLOS")
                             );


    sania_token_balance = get_account(N(sania), "4,GOLOS");
    REQUIRE_MATCHING_OBJECT( sania_token_balance, mvo()
                             ("balance", "410.0000 GOLOS") );
} FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE( test_cancel_convert_vesting_to_token, golos_vesting_tester ) try {
    BOOST_REQUIRE_EQUAL( success(), create(N(golos.issuer), asset::from_string("100000.0000 GOLOS")) );
    BOOST_REQUIRE_EQUAL( success(), issue(N(golos.issuer), N(sania), asset::from_string("500.0000 GOLOS"), "issue tokens sania") );
    BOOST_REQUIRE_EQUAL( success(), issue(N(golos.issuer), N(pasha), asset::from_string("500.0000 GOLOS"), "issue tokens pasha") );
    produce_blocks(1);

    BOOST_REQUIRE_EQUAL( success(), open_balance(N(sania), symbol(4,"GOLOS"), N(sania)) );
    BOOST_REQUIRE_EQUAL( success(), open_balance(N(pasha), symbol(4,"GOLOS"), N(pasha)) );
    produce_blocks(1);

    BOOST_REQUIRE_EQUAL( success(), create_vesting_token(N(golos.issuer), symbol(4,"GOLOS")) );
    BOOST_REQUIRE_EQUAL( success(), transfer(N(sania), N(golos.vest), asset::from_string("100.0000 GOLOS"), "convert token to vesting") );
    BOOST_REQUIRE_EQUAL( success(), transfer(N(pasha), N(golos.vest), asset::from_string("100.0000 GOLOS"), "convert token to vesting") );
    produce_blocks(1);

    auto sania_token_balance = get_account(N(sania), "4,GOLOS");
    REQUIRE_MATCHING_OBJECT( sania_token_balance, mvo()
                             ("balance", "400.0000 GOLOS") );

    auto pasha_token_balance = get_account(N(pasha), "4,GOLOS");
    REQUIRE_MATCHING_OBJECT( pasha_token_balance, mvo()
                             ("balance", "400.0000 GOLOS") );

    auto sania_vesting_balance = get_account_vesting(N(sania), "4,GOLOS");
    REQUIRE_MATCHING_OBJECT( sania_vesting_balance, mvo()
                             ("vesting", "100.0000 GOLOS")
                             ("delegate_vesting", "0.0000 GOLOS")
                             ("received_vesting", "0.0000 GOLOS")
                             ("unlocked_limit", "0.0000 GOLOS")
                             );

    auto pasha_vesting_balance = get_account_vesting(N(pasha), "4,GOLOS");
    REQUIRE_MATCHING_OBJECT( pasha_vesting_balance, mvo()
                             ("vesting", "100.0000 GOLOS")
                             ("delegate_vesting", "0.0000 GOLOS")
                             ("received_vesting", "0.0000 GOLOS")
                             ("unlocked_limit", "0.0000 GOLOS")
                             );


    BOOST_REQUIRE_EQUAL( success(), convert_vesting(N(sania), N(sania), asset::from_string("13.0000 GOLOS")) );
    BOOST_REQUIRE_EQUAL( success(), start_timer_trx() );
    auto delegated_auth = authority( 1, {}, {
                                         { .permission = {N(golos.vest), config::eosio_code_name}, .weight = 1}
                                     });
    set_authority( N(golos.vest),  config::active_name,  delegated_auth );
    produce_blocks(31);

    sania_vesting_balance = get_account_vesting(N(sania), "4,GOLOS");
    REQUIRE_MATCHING_OBJECT( sania_vesting_balance, mvo()
                             ("vesting", "99.0000 GOLOS")
                             ("delegate_vesting", "0.0000 GOLOS")
                             ("received_vesting", "0.0000 GOLOS")
                             ("unlocked_limit", "0.0000 GOLOS")
                             );

    sania_token_balance = get_account(N(sania), "4,GOLOS");
    REQUIRE_MATCHING_OBJECT( sania_token_balance, mvo()
                             ("balance", "401.0000 GOLOS") );

    produce_blocks(120);
    sania_vesting_balance = get_account_vesting(N(sania), "4,GOLOS");
    REQUIRE_MATCHING_OBJECT( sania_vesting_balance, mvo()
                             ("vesting", "95.0000 GOLOS")
                             ("delegate_vesting", "0.0000 GOLOS")
                             ("received_vesting", "0.0000 GOLOS")
                             ("unlocked_limit", "0.0000 GOLOS")
                             );

    sania_token_balance = get_account(N(sania), "4,GOLOS");
    REQUIRE_MATCHING_OBJECT( sania_token_balance, mvo()
                             ("balance", "405.0000 GOLOS") );

    BOOST_REQUIRE_EQUAL( success(), cancel_convert_vesting(N(sania), asset::from_string("0.0000 GOLOS")) );
    produce_blocks(120);

    sania_vesting_balance = get_account_vesting(N(sania), "4,GOLOS");
    REQUIRE_MATCHING_OBJECT( sania_vesting_balance, mvo()
                             ("vesting", "95.0000 GOLOS")
                             ("delegate_vesting", "0.0000 GOLOS")
                             ("received_vesting", "0.0000 GOLOS")
                             ("unlocked_limit", "0.0000 GOLOS")
                             );

    sania_token_balance = get_account(N(sania), "4,GOLOS");
    REQUIRE_MATCHING_OBJECT( sania_token_balance, mvo()
                             ("balance", "405.0000 GOLOS") );
} FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE( error_sender_equals_test_delegate_vesting, golos_vesting_tester ) try {
    prepare_balances();

    BOOST_REQUIRE_EQUAL( error("assertion failure with message: You can not delegate to yourself"), delegate_vesting(N(sania), N(sania), asset::from_string("15.0000 GOLOS"), 0, 0) );
} FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE( error_payout_strategy_test_delegate_vesting, golos_vesting_tester ) try {
    prepare_balances();

    BOOST_REQUIRE_EQUAL( error("assertion failure with message: not valid value payout_strategy"), delegate_vesting(N(sania), N(pasha), asset::from_string("15.0000 GOLOS"), 0, -1) );
    BOOST_REQUIRE_EQUAL( error("assertion failure with message: not valid value payout_strategy"), delegate_vesting(N(sania), N(pasha), asset::from_string("15.0000 GOLOS"), 0, 2) );
} FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE( error_zero_quantity_test_delegate_vesting, golos_vesting_tester ) try {
    prepare_balances();
    BOOST_REQUIRE_EQUAL( error("assertion failure with message: the number of tokens should not be less than 0"), delegate_vesting(N(sania), N(pasha), asset::from_string("0.0000 GOLOS"), 0, 0) );
} FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE( error_min_amount_delegate_test_delegate_vesting, golos_vesting_tester ) try {
    prepare_balances();

    BOOST_REQUIRE_EQUAL( error("assertion failure with message: Insufficient funds for delegation"), delegate_vesting(N(sania), N(pasha), asset::from_string("0.0001 GOLOS"), 0, 0) );
} FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE( error_interest_rate_test_delegate_vesting, golos_vesting_tester ) try {
    prepare_balances();

    BOOST_REQUIRE_EQUAL( error("assertion failure with message: Exceeded the percentage of delegated vesting"), delegate_vesting(N(sania), N(pasha), asset::from_string("15.0000 GOLOS"), 50, 0) );
} FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE( error_insufficient_funds_test_delegate_vesting, golos_vesting_tester ) try {
    prepare_balances();

    BOOST_REQUIRE_EQUAL( success(), delegate_vesting(N(sania), N(tanya), asset::from_string("15.0000 GOLOS"), 0, 0) );
    BOOST_REQUIRE_EQUAL( success(), convert_vesting(N(sania), N(sania), asset::from_string("80.0000 GOLOS")) );
    BOOST_REQUIRE_EQUAL( error("assertion failure with message: insufficient funds for delegation"), delegate_vesting(N(sania), N(pasha), asset::from_string("15.0000 GOLOS"), 0, 0) );

} FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE( test_delegate_vesting, golos_vesting_tester ) try {
    prepare_balances();

    BOOST_REQUIRE_EQUAL( success(), delegate_vesting(N(sania), N(pasha), asset::from_string("15.0000 GOLOS"), 0, 0) );

    auto pasha_vesting_balance_delegate = get_account_vesting(N(pasha), "4,GOLOS");
    REQUIRE_MATCHING_OBJECT( pasha_vesting_balance_delegate, mvo()
                             ("vesting", "100.0000 GOLOS")
                             ("delegate_vesting", "0.0000 GOLOS")
                             ("received_vesting", "15.0000 GOLOS")
                             ("unlocked_limit", "0.0000 GOLOS")
                             );

    auto sania_vesting_balance_delegate = get_account_vesting(N(sania), "4,GOLOS");
    REQUIRE_MATCHING_OBJECT( sania_vesting_balance_delegate, mvo()
                             ("vesting", "100.0000 GOLOS")
                             ("delegate_vesting", "15.0000 GOLOS")
                             ("received_vesting", "0.0000 GOLOS")
                             ("unlocked_limit", "0.0000 GOLOS")
                             );
} FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE( error_insufficient_funds_test_undelegate_vesting, golos_vesting_tester ) try {
    prepare_balances();

    BOOST_REQUIRE_EQUAL( success(), delegate_vesting(N(sania), N(pasha), asset::from_string("20.0000 GOLOS"), 0, 1) );

    auto pasha_vesting_balance_delegate = get_account_vesting(N(pasha), "4,GOLOS");
    REQUIRE_MATCHING_OBJECT( pasha_vesting_balance_delegate, mvo()
                             ("vesting", "100.0000 GOLOS")
                             ("delegate_vesting", "0.0000 GOLOS")
                             ("received_vesting", "20.0000 GOLOS")
                             ("unlocked_limit", "0.0000 GOLOS")
                             );

    auto sania_vesting_balance_delegate = get_account_vesting(N(sania), "4,GOLOS");
    REQUIRE_MATCHING_OBJECT( sania_vesting_balance_delegate, mvo()
                             ("vesting", "100.0000 GOLOS")
                             ("delegate_vesting", "20.0000 GOLOS")
                             ("received_vesting", "0.0000 GOLOS")
                             ("unlocked_limit", "0.0000 GOLOS")
                             );

    produce_blocks(100);

    BOOST_REQUIRE_EQUAL( error("assertion failure with message: Insufficient funds for undelegation"), undelegate_vesting(N(sania), N(pasha), asset::from_string("4.0000 GOLOS")) );
} FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE( error_frozen_tokens_test_undelegate_vesting, golos_vesting_tester ) try {
    prepare_balances();

    BOOST_REQUIRE_EQUAL( success(), delegate_vesting(N(sania), N(pasha), asset::from_string("20.0000 GOLOS"), 0, 1) );

    auto pasha_vesting_balance_delegate = get_account_vesting(N(pasha), "4,GOLOS");
    REQUIRE_MATCHING_OBJECT( pasha_vesting_balance_delegate, mvo()
                             ("vesting", "100.0000 GOLOS")
                             ("delegate_vesting", "0.0000 GOLOS")
                             ("received_vesting", "20.0000 GOLOS")
                             ("unlocked_limit", "0.0000 GOLOS")
                             );

    auto sania_vesting_balance_delegate = get_account_vesting(N(sania), "4,GOLOS");
    REQUIRE_MATCHING_OBJECT( sania_vesting_balance_delegate, mvo()
                             ("vesting", "100.0000 GOLOS")
                             ("delegate_vesting", "20.0000 GOLOS")
                             ("received_vesting", "0.0000 GOLOS")
                             ("unlocked_limit", "0.0000 GOLOS")
                             );
    BOOST_REQUIRE_EQUAL( error("assertion failure with message: Tokens are frozen until the end of the period"), undelegate_vesting(N(sania), N(pasha), asset::from_string("5.0000 GOLOS")) );
} FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE( error_lack_of_funds_test_undelegate_vesting, golos_vesting_tester ) try {
    prepare_balances();

    BOOST_REQUIRE_EQUAL( success(), delegate_vesting(N(sania), N(pasha), asset::from_string("20.0000 GOLOS"), 0, 1) );

    auto pasha_vesting_balance_delegate = get_account_vesting(N(pasha), "4,GOLOS");
    REQUIRE_MATCHING_OBJECT( pasha_vesting_balance_delegate, mvo()
                             ("vesting", "100.0000 GOLOS")
                             ("delegate_vesting", "0.0000 GOLOS")
                             ("received_vesting", "20.0000 GOLOS")
                             ("unlocked_limit", "0.0000 GOLOS")
                             );

    auto sania_vesting_balance_delegate = get_account_vesting(N(sania), "4,GOLOS");
    REQUIRE_MATCHING_OBJECT( sania_vesting_balance_delegate, mvo()
                             ("vesting", "100.0000 GOLOS")
                             ("delegate_vesting", "20.0000 GOLOS")
                             ("received_vesting", "0.0000 GOLOS")
                             ("unlocked_limit", "0.0000 GOLOS")
                             );

    produce_blocks(100);

    BOOST_REQUIRE_EQUAL( error("assertion failure with message: There are not enough delegated tools for output"),
                         undelegate_vesting(N(sania), N(pasha), asset::from_string("24.0000 GOLOS")) );
} FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE( error_delegated_vesting_withdrawn_test_undelegate_vesting, golos_vesting_tester ) try {
    prepare_balances();

    BOOST_REQUIRE_EQUAL( success(), delegate_vesting(N(sania), N(pasha), asset::from_string("20.0000 GOLOS"), 0, 1) );

    auto pasha_vesting_balance_delegate = get_account_vesting(N(pasha), "4,GOLOS");
    REQUIRE_MATCHING_OBJECT( pasha_vesting_balance_delegate, mvo()
                             ("vesting", "100.0000 GOLOS")
                             ("delegate_vesting", "0.0000 GOLOS")
                             ("received_vesting", "20.0000 GOLOS")
                             ("unlocked_limit", "0.0000 GOLOS")
                             );

    auto sania_vesting_balance_delegate = get_account_vesting(N(sania), "4,GOLOS");
    REQUIRE_MATCHING_OBJECT( sania_vesting_balance_delegate, mvo()
                             ("vesting", "100.0000 GOLOS")
                             ("delegate_vesting", "20.0000 GOLOS")
                             ("received_vesting", "0.0000 GOLOS")
                             ("unlocked_limit", "0.0000 GOLOS")
                             );

    produce_blocks(100);

    BOOST_REQUIRE_EQUAL( error("assertion failure with message: delegated vesting withdrawn"),
                         undelegate_vesting(N(sania), N(pasha), asset::from_string("18.0000 GOLOS")) );
} FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE( test_undelegate_vesting, golos_vesting_tester ) try {
    prepare_balances();

    BOOST_REQUIRE_EQUAL( success(), delegate_vesting(N(sania), N(pasha), asset::from_string("20.0000 GOLOS"), 0, 1) );

    auto pasha_vesting_balance_delegate = get_account_vesting(N(pasha), "4,GOLOS");
    REQUIRE_MATCHING_OBJECT( pasha_vesting_balance_delegate, mvo()
                             ("vesting", "100.0000 GOLOS")
                             ("delegate_vesting", "0.0000 GOLOS")
                             ("received_vesting", "20.0000 GOLOS")
                             ("unlocked_limit", "0.0000 GOLOS")
                             );

    auto sania_vesting_balance_delegate = get_account_vesting(N(sania), "4,GOLOS");
    REQUIRE_MATCHING_OBJECT( sania_vesting_balance_delegate, mvo()
                             ("vesting", "100.0000 GOLOS")
                             ("delegate_vesting", "20.0000 GOLOS")
                             ("received_vesting", "0.0000 GOLOS")
                             ("unlocked_limit", "0.0000 GOLOS")
                             );

    produce_blocks(100);

    BOOST_REQUIRE_EQUAL( success(), undelegate_vesting(N(sania), N(pasha), asset::from_string("5.0000 GOLOS")) );

    auto pasha_vesting_balance_undelegate = get_account_vesting(N(pasha), "4,GOLOS");
    REQUIRE_MATCHING_OBJECT( pasha_vesting_balance_undelegate, mvo()
                             ("vesting", "100.0000 GOLOS")
                             ("delegate_vesting", "0.0000 GOLOS")
                             ("received_vesting", "15.0000 GOLOS")
                             ("unlocked_limit", "0.0000 GOLOS")
                             );

    auto sania_vesting_balance_undelegate = get_account_vesting(N(sania), "4,GOLOS");
    REQUIRE_MATCHING_OBJECT( sania_vesting_balance_undelegate, mvo()
                             ("vesting", "100.0000 GOLOS")
                             ("delegate_vesting", "20.0000 GOLOS")
                             ("received_vesting", "0.0000 GOLOS")
                             ("unlocked_limit", "0.0000 GOLOS")
                             );

    start_timer_trx();
    auto delegated_auth = authority( 1, {}, {
                                         { .permission = {N(golos.vest), config::eosio_code_name}, .weight = 1}
                                     });
    set_authority( N(golos.vest),  config::active_name,  delegated_auth );
    produce_blocks(31);

    sania_vesting_balance_delegate = get_account_vesting(N(sania), "4,GOLOS");
    REQUIRE_MATCHING_OBJECT( sania_vesting_balance_delegate, mvo()
                             ("vesting", "100.0000 GOLOS")
                             ("delegate_vesting", "15.0000 GOLOS")
                             ("received_vesting", "0.0000 GOLOS")
                             ("unlocked_limit", "0.0000 GOLOS")
                             );

    pasha_vesting_balance_delegate = get_account_vesting(N(pasha), "4,GOLOS");
    REQUIRE_MATCHING_OBJECT( pasha_vesting_balance_delegate, mvo()
                             ("vesting", "100.0000 GOLOS")
                             ("delegate_vesting", "0.0000 GOLOS")
                             ("received_vesting", "15.0000 GOLOS")
                             ("unlocked_limit", "0.0000 GOLOS")
                             );

} FC_LOG_AND_RETHROW()

BOOST_AUTO_TEST_SUITE_END()
