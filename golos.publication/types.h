#pragma once

using enum_t = uint8_t;
using base_t = int64_t;// !if you change it -- don't forget about the abi-file
constexpr int fixed_point_fractional_digits = 12;

struct
#ifndef UNIT_TEST_ENV
[[eosio::table]] 
#endif
funcparams {
    std::string str;
    base_t maxarg;
};

constexpr uint8_t disabled_restorer = std::numeric_limits<uint8_t>::max();
struct 
#ifndef UNIT_TEST_ENV
[[eosio::table]] 
#endif
limitedact {
    uint8_t chargenum;
    uint8_t restorernum;
    base_t cutoffval;
    base_t chargeprice;
};

struct limitsarg {
    std::vector<std::string> restorers;
    std::vector<limitedact> limitedacts;
    std::vector<int64_t> vestingprices;
    std::vector<int64_t> minvestings;
};

#ifdef UNIT_TEST_ENV
FC_REFLECT(limitedact, (chargenum)(restorernum)(cutoffval)(chargeprice))
FC_REFLECT(limitsarg, (restorers)(limitedacts)(vestingprices)(minvestings))
#endif

enum class payment_t: enum_t { TOKEN, VESTING };

#if defined(UNIT_TEST_ENV)
#include "../common/calclib/fixed_point_utils.h"
#else
#include <common/calclib/fixed_point_utils.h>
#endif




