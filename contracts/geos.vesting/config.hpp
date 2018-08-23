#pragma once

#define NULL_TOKEN_VESTING asset(0, string_to_symbol(4, "VEST"))
#define NULL_TOKEN_GOLOS asset(0, string_to_symbol(4, "GOLOS"))
#define TOKEN_GOLOS(amount) asset(int64_t(amount), string_to_symbol(4, "GOLOS"))


// TODO test params
#define OUTPUT_WEEK_COUNT 10
#define OUTPUT_PAYOUT_PERIOD 15

//#define OUTPUT_WEEK_COUNT 13
//#define OUTPUT_PAYOUT_PERIOD 604800

#define TIMEOUT 3
#define WEEK_PERIOD 1
#define NOT_TOKENS_WEEK_PERIOD 0
