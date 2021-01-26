#ifndef STATICS_HPP
#define STATICS_HPP

#include <map>
#include <set>
#include <string>

#define MHC *(1000l * 1000l)

// BLOCK TYPE's
const uint64_t BLOCK_BAD = 0x6745230100000000;
const uint64_t BLOCK_GOOD = 0x6745230111111111;
const uint64_t BLOCK_UGLY = 0x6745230122222222;

const uint64_t BLOCK_TYPE = 0xEFCDAB8967452301;

const uint64_t BLOCK_TYPE_COMMON = 0x0001000067452301;
const uint64_t BLOCK_TYPE_STATE = 0x1101000067452301;
const uint64_t BLOCK_TYPE_FORGING = 0x2201000067452301;

const uint64_t BLOCK_TYPE_TECH_APPROVE = 0x1100111167452301;
const uint64_t BLOCK_TYPE_TECH_STATS = 0x2200111167452301;
const uint64_t BLOCK_TYPE_TECH_BAD_TX = 0x3300111167452301;

// SPECIAL WALLETS
const std::string MASTER_WALLET_COIN_FORGING = "0x666174686572206f662077616c6c65747320666f7267696e67";
const std::string MASTER_WALLET_NODE_FORGING = "0x666174686572206f662073657276657220666f7267696e6720";
const std::string SPECIAL_WALLET_COMISSIONS = "0x5350454349414c5f57414c4c45545f434f4d495353494f4e53";
const std::string STATE_FEE_WALLET = "0x7374617465206665652061732077616c6c6574206164726573";
const std::string ZERO_WALLET = "0x00000000000000000000000000000000000000000000000000";

const std::set<std::string> FOUNDER_WALLETS {
    "0x007c5d7a487a3fea657b581b50b809bf878f418cdf346ee352",
    "0x009f684260a15920cb5200cadf766455305cc87f04b360a756",
    "0x00c6ab64013d614efdc90243fdef100da14ddee4c482ca9c7b"
};

const std::string TEAM_WALLET = "0x00e3aafe3399cdb92a6780cc8d2e4b38c4429ffc41c8c57d6b";

//const std::set<std::string> FOUNDER_WALLETS {
//    "0x007c5d7a487a3fea657b581b50b809bf878f418cdf346ee352",
//    "0x009f684260a15920cb5200cadf766455305cc87f04b360a756",
//    "0x00c6ab64013d614efdc90243fdef100da14ddee4c482ca9c7b"
//};

// DELEGATION LIMITS
const uint64_t LIMIT_DELEGATE_TO = 256;
const uint64_t LIMIT_DELEGATE_FROM = 4096;

const uint64_t MINIMUM_COIN_FORGING_W = 100 MHC;
const uint64_t MINIMUM_COIN_FORGING_C = 512 MHC;
const uint64_t MINIMUM_COIN_FORGING_N = 512 MHC;

const uint64_t DECADE_POOL = 4'600'000'000 MHC;
//            start ts    pool
const std::map<uint64_t, uint64_t> FORGING_POOL_PER_YEAR {
    /*2019*/ { 1550000000, 1940822 MHC },
    /*2020*/ { 1581900000, (DECADE_POOL * 138 / 1000) / 366 },
    /*2021*/ { 1613200000, (DECADE_POOL * 124 / 1000) / 365 },
    /*2022*/ { 1644790000, (DECADE_POOL * 112 / 1000) / 365 },
    /*2023*/ { 1676300000, (DECADE_POOL * 101 / 1000) / 365 },
    /*2024*/ { 1707800000, (DECADE_POOL * 91 / 1000) / 366 },
    /*2025*/ { 1739490000, (DECADE_POOL * 82 / 1000) / 365 },
    /*2026*/ { 1771000000, (DECADE_POOL * 73 / 1000) / 365 },
    /*2027*/ { 1802500000, (DECADE_POOL * 66 / 1000) / 365 },
    /*2028*/ { 1834090000, (DECADE_POOL * 59 / 1000) / 366 },
    /*2029+*/ { 1865800000, 125000 MHC },
    /*2039+*/ { 2181300000, 0 MHC }
};

const uint64_t FORGING_TEAM_REWARD = 230'000 MHC;
const uint64_t FOUNDER_DAILY_LIMIT_UP = 230'000 MHC;
const uint64_t FOUNDER_INITIAL_LIMIT = 20'930'000 MHC;

// COMISSIONS FOR TRANSACTIONS
const uint64_t COMISSION_COMMON_00_20 = 0 MHC;
const uint64_t COMISSION_COMMON_21_40 = 1 MHC;
const uint64_t COMISSION_COMMON_41_60 = 10 MHC;
const uint64_t COMISSION_COMMON_61_80 = 50 MHC;
const uint64_t COMISSION_COMMON_81_99 = 100 MHC;

const uint64_t COMISSION_JSON_00_20 = 1 MHC;
const uint64_t COMISSION_JSON_21_40 = 10 MHC;
const uint64_t COMISSION_JSON_41_60 = 30 MHC;
const uint64_t COMISSION_JSON_61_80 = 100 MHC;
const uint64_t COMISSION_JSON_81_99 = 1000 MHC;

const uint64_t COMISSION_DATA_00_20 = 1 MHC;
const uint64_t COMISSION_DATA_21_40 = 30 MHC;
const uint64_t COMISSION_DATA_41_60 = 60 MHC;
const uint64_t COMISSION_DATA_61_80 = 200 MHC;
const uint64_t COMISSION_DATA_81_99 = 2000 MHC;

const uint64_t MAX_TRANSACTION_COUNT = 50 * 1024;

// TX STATE
const uint64_t TX_STATE_APPROVE = 1;
const uint64_t TX_STATE_FEE = 2;
const uint64_t TX_STATE_ACCEPT = 20;
const uint64_t TX_STATE_WRONG_DATA = 40;
const uint64_t TX_STATE_FORGING = 100;
const uint64_t TX_STATE_FORGING_W = 101;
const uint64_t TX_STATE_FORGING_N = 102;
const uint64_t TX_STATE_FORGING_C = 103;
const uint64_t TX_STATE_FORGING_R = 104;
const uint64_t TX_STATE_FORGING_DAPP = 110;
const uint64_t TX_STATE_FORGING_TEAM = 127;
const uint64_t TX_STATE_FORGING_FOUNDER = 128;
const uint64_t TX_STATE_STATE = 200;
const uint64_t TX_STATE_TECH_NODE_STAT = 0x1101;

// TX REJECT REASON
const uint64_t TX_REJECT_ZERO = 0xff01;
const uint64_t TX_REJECT_INSUFFICIENT_FEE = 0xff02;
const uint64_t TX_REJECT_INSUFFICIENT_FUNDS = 0xff03;
const uint64_t TX_REJECT_INVALID_NONCE = 0xff04;
const uint64_t TX_REJECT_INSUFFICIENT_FUNDS_EXT = 0xff05;
const uint64_t TX_REJECT_FOUNDER_LIMIT = 0xff06;
const uint64_t TX_REJECT_INVALID_WALLET = 0x0404;

const uint64_t DAY_IN_SECONDS = 24 * 60 * 60;
const uint64_t CORE_LIST_RENEW_PERIOD = 10 * 60;
const uint64_t CORE_LIST_SILENCE_PERIOD = 30;

/*                          RPC METHODS                           */
const uint64_t RPC_TX = 0x20;
const uint64_t RPC_PRETEND_BLOCK = 0x30;
const uint64_t RPC_APPROVE = 0x40;
const uint64_t RPC_DISAPPROVE = 0x41;
const uint64_t RPC_GET_APPROVE = 0x42;
const uint64_t RPC_LAST_BLOCK = 0x50;
const uint64_t RPC_GET_BLOCK = 0x60;
const uint64_t RPC_GET_CHAIN = 0x61;
const uint64_t RPC_GET_MISSING_BLOCK_LIST = 0x62;
const uint64_t RPC_GET_CORE_LIST = 0x71;
const uint64_t RPC_CORE_LIST_APPROVE = 0x81;

/*                           NODE ROLE CONSTANTS                           */
const uint64_t METAHASH_PRIMARY_CORES_COUNT = 4;

static const std::string META_ROLE_CORE = "Core";
static const std::string META_ROLE_PROXY = "Proxy";
static const std::string META_ROLE_VERIF = "Verifier";
static const std::string META_ROLE_TORR = "Torrent";
static const std::string META_ROLE_I_TORR = "InfrastructureTorrent";
static const std::set<std::string> ROLES {
    META_ROLE_PROXY,
    META_ROLE_TORR,
    META_ROLE_I_TORR,
    META_ROLE_VERIF,
    META_ROLE_CORE
};

const uint64_t MINIMUM_AVERAGE_PROXY_RPS = 1000;
const uint64_t MINIMUM_PROXY_RPS = 0;

const uint64_t NODE_PROXY_SEED_CAP = 0 MHC;
const uint64_t NODE_PROXY_SOFT_CAP = 100'000 MHC;
//TODO 1'000'000 MHC;
const uint64_t NODE_PROXY_HARD_CAP = 10'000'000 MHC;
const uint64_t NODE_INFRASTRUCTURETORRENT_SEED_CAP = 100'000 MHC;
const uint64_t NODE_INFRASTRUCTURETORRENT_SOFT_CAP = 500'000 MHC;
//TODO 2'000'000 MHC;
const uint64_t NODE_INFRASTRUCTURETORRENT_HARD_CAP = 20'000'000 MHC;
const uint64_t NODE_TORRENT_SEED_CAP = 100'000 MHC;
const uint64_t NODE_TORRENT_SOFT_CAP = 1'000'000 MHC;
const uint64_t NODE_TORRENT_HARD_CAP = 20'000'000 MHC;
const uint64_t NODE_VERIFIER_SEED_CAP = 100'000 MHC;
const uint64_t NODE_VERIFIER_SOFT_CAP = 1'000'000 MHC;
const uint64_t NODE_VERIFIER_HARD_CAP = 10'000'000 MHC;
const uint64_t NODE_CORE_SEED_CAP = 500'000 MHC;
const uint64_t NODE_CORE_SOFT_CAP = 10'000'000 MHC;
const uint64_t NODE_CORE_HARD_CAP = 100'000'000 MHC;

// WALLET STATE
const uint64_t NODE_STATE_FLAG_PRETEND_COMMON = 0b0001;

const uint64_t NODE_STATE_FLAG_PROXY_PRETEND = 0b0000'0000'0001'0000;
const uint64_t NODE_STATE_FLAG_PROXY_SEED_CAP = 0b0000'0000'0010'0000;
const uint64_t NODE_STATE_FLAG_PROXY_SOFT_CAP = 0b0000'0000'0100'0000;
const uint64_t NODE_STATE_FLAG_PROXY_FORGING = NODE_STATE_FLAG_PROXY_PRETEND | NODE_STATE_FLAG_PROXY_SEED_CAP | NODE_STATE_FLAG_PROXY_SOFT_CAP | NODE_STATE_FLAG_PRETEND_COMMON;

const uint64_t NODE_STATE_FLAG_INFRASTRUCTURETORRENT_PRETEND = 0b0000'0001'0000'0000;
const uint64_t NODE_STATE_FLAG_INFRASTRUCTURETORRENT_SEED_CAP = 0b0000'0010'0000'0000;
const uint64_t NODE_STATE_FLAG_INFRASTRUCTURETORRENT_SOFT_CAP = 0b0000'0100'0000'0000;
const uint64_t NODE_STATE_FLAG_INFRASTRUCTURETORRENT_FORGING = NODE_STATE_FLAG_INFRASTRUCTURETORRENT_PRETEND | NODE_STATE_FLAG_INFRASTRUCTURETORRENT_SEED_CAP | NODE_STATE_FLAG_INFRASTRUCTURETORRENT_SOFT_CAP | NODE_STATE_FLAG_PRETEND_COMMON;

const uint64_t NODE_STATE_FLAG_TORRENT_PRETEND = 0b0000'0001'0000'0000'0000;
const uint64_t NODE_STATE_FLAG_TORRENT_SEED_CAP = 0b0000'0010'0000'0000'0000;
const uint64_t NODE_STATE_FLAG_TORRENT_SOFT_CAP = 0b0000'0100'0000'0000'0000;
const uint64_t NODE_STATE_FLAG_TORRENT_FORGING = NODE_STATE_FLAG_TORRENT_PRETEND | NODE_STATE_FLAG_TORRENT_SEED_CAP | NODE_STATE_FLAG_TORRENT_SOFT_CAP | NODE_STATE_FLAG_PRETEND_COMMON;

const uint64_t NODE_STATE_FLAG_VERIFIER_PRETEND = 0b0000'0001'0000'0000'0000'0000;
const uint64_t NODE_STATE_FLAG_VERIFIER_SEED_CAP = 0b0000'0010'0000'0000'0000'0000;
const uint64_t NODE_STATE_FLAG_VERIFIER_SOFT_CAP = 0b0000'0100'0000'0000'0000'0000;
const uint64_t NODE_STATE_FLAG_VERIFIER_FORGING = NODE_STATE_FLAG_VERIFIER_PRETEND | NODE_STATE_FLAG_VERIFIER_SEED_CAP | NODE_STATE_FLAG_VERIFIER_SOFT_CAP | NODE_STATE_FLAG_PRETEND_COMMON;

const uint64_t NODE_STATE_FLAG_CORE_PRETEND = 0b0000'0001'0000'0000'0000'0000'0000;
const uint64_t NODE_STATE_FLAG_CORE_SEED_CAP = 0b0000'0010'0000'0000'0000'0000'0000;
const uint64_t NODE_STATE_FLAG_CORE_SOFT_CAP = 0b0000'0100'0000'0000'0000'0000'0000;
const uint64_t NODE_STATE_FLAG_CORE_FORGING = NODE_STATE_FLAG_CORE_PRETEND | NODE_STATE_FLAG_CORE_SEED_CAP | NODE_STATE_FLAG_CORE_SOFT_CAP | NODE_STATE_FLAG_PRETEND_COMMON;

static const std::map<std::string, uint64_t> NODE_SEED_CAP {
    { "Proxy", NODE_PROXY_SEED_CAP },
    { "InfrastructureTorrent", NODE_INFRASTRUCTURETORRENT_SEED_CAP },
    { "Torrent", NODE_TORRENT_SEED_CAP },
    { "Verifier", NODE_VERIFIER_SEED_CAP },
    { "Core", NODE_CORE_SEED_CAP }
};
static const std::map<std::string, uint64_t> NODE_SOFT_CAP {
    { "Proxy", NODE_PROXY_SOFT_CAP },
    { "InfrastructureTorrent", NODE_INFRASTRUCTURETORRENT_SOFT_CAP },
    { "Torrent", NODE_TORRENT_SOFT_CAP },
    { "Verifier", NODE_VERIFIER_SOFT_CAP },
    { "Core", NODE_CORE_SOFT_CAP }
};
static const std::map<std::string, uint64_t> NODE_HARD_CAP {
    { "Proxy", NODE_PROXY_HARD_CAP },
    { "InfrastructureTorrent", NODE_INFRASTRUCTURETORRENT_HARD_CAP },
    { "Torrent", NODE_TORRENT_HARD_CAP },
    { "Verifier", NODE_VERIFIER_HARD_CAP },
    { "Core", NODE_CORE_HARD_CAP }
};
static const std::map<std::string, uint64_t> NODE_STATE_FLAG_PRETEND {
    { "Proxy", NODE_STATE_FLAG_PROXY_PRETEND },
    { "InfrastructureTorrent", NODE_STATE_FLAG_INFRASTRUCTURETORRENT_PRETEND },
    { "Torrent", NODE_STATE_FLAG_TORRENT_PRETEND },
    { "Verifier", NODE_STATE_FLAG_VERIFIER_PRETEND },
    { "Core", NODE_STATE_FLAG_CORE_PRETEND }
};
static const std::map<std::string, uint64_t> NODE_STATE_FLAG_SEED_CAP {
    { "Proxy", NODE_STATE_FLAG_PROXY_SEED_CAP },
    { "InfrastructureTorrent", NODE_STATE_FLAG_INFRASTRUCTURETORRENT_SEED_CAP },
    { "Torrent", NODE_STATE_FLAG_TORRENT_SEED_CAP },
    { "Verifier", NODE_STATE_FLAG_VERIFIER_SEED_CAP },
    { "Core", NODE_STATE_FLAG_CORE_SEED_CAP }
};
static const std::map<std::string, uint64_t> NODE_STATE_FLAG_SOFT_CAP {
    { "Proxy", NODE_STATE_FLAG_PROXY_SOFT_CAP },
    { "InfrastructureTorrent", NODE_STATE_FLAG_INFRASTRUCTURETORRENT_SOFT_CAP },
    { "Torrent", NODE_STATE_FLAG_TORRENT_SOFT_CAP },
    { "Verifier", NODE_STATE_FLAG_VERIFIER_SOFT_CAP },
    { "Core", NODE_STATE_FLAG_CORE_SOFT_CAP }
};
static const std::map<std::string, uint64_t> NODE_STATE_FLAG_FORGING {
    { "Proxy", NODE_STATE_FLAG_PROXY_FORGING },
    { "InfrastructureTorrent", NODE_STATE_FLAG_INFRASTRUCTURETORRENT_FORGING },
    { "Torrent", NODE_STATE_FLAG_TORRENT_FORGING },
    { "Verifier", NODE_STATE_FLAG_VERIFIER_FORGING },
    { "Core", NODE_STATE_FLAG_CORE_FORGING }
};

#endif // STATICS_HPP
