#include "transaction.h"
#include "open_ssl_decor.h"

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>

#include <iostream>

#include <meta_log.hpp>
#include <statics.hpp>

namespace metahash::metachain {

TX::TX() = default;

TX::TX(const TX& other)
    : state(other.state)
    , tx_size(other.tx_size)
    , value(other.value)
    , fee(other.fee)
    , nonce(other.nonce)
{

    bin_to = other.bin_to;
    data = other.data;
    sign = other.sign;
    pub_key = other.pub_key;

    data_for_sign = other.data_for_sign;

    raw_tx = other.raw_tx;

    hash = other.hash;

    addr_from = other.addr_from;
    addr_to = other.addr_to;

    if (other.json_rpc) {
        json_rpc = new JSON_RPC;
        *json_rpc = *(other.json_rpc);
    }
}

TX::TX(TX&& other)
    : state(other.state)
    , tx_size(other.tx_size)
    , value(other.value)
    , fee(other.fee)
    , nonce(other.nonce)
{
    bin_to = std::move(other.bin_to);
    data = std::move(other.data);
    sign = std::move(other.sign);
    pub_key = std::move(other.pub_key);

    data_for_sign = std::move(other.data_for_sign);

    raw_tx = std::move(other.raw_tx);

    hash = std::move(other.hash);

    addr_from = std::move(other.addr_from);
    addr_to = std::move(other.addr_to);

    json_rpc = other.json_rpc;
    other.json_rpc = nullptr;
}

TX& TX::operator=(const TX& other)
{
    if (this != &other) {
        state = other.state;
        tx_size = other.tx_size;
        value = other.value;
        fee = other.fee;
        nonce = other.nonce;

        bin_to = other.bin_to;
        data = other.data;
        sign = other.sign;
        pub_key = other.pub_key;

        data_for_sign = other.data_for_sign;

        raw_tx = other.raw_tx;

        hash = other.hash;

        addr_from = other.addr_from;
        addr_to = other.addr_to;

        if (other.json_rpc) {
            json_rpc = new JSON_RPC;
            *json_rpc = *(other.json_rpc);
        }
    }

    return *this;
}

TX& TX::operator=(TX&& other)
{
    if (this != &other) {
        state = other.state;
        tx_size = other.tx_size;
        value = other.value;
        fee = other.fee;
        nonce = other.nonce;

        bin_to = std::move(other.bin_to);
        data = std::move(other.data);
        sign = std::move(other.sign);
        pub_key = std::move(other.pub_key);

        data_for_sign = std::move(other.data_for_sign);

        raw_tx = std::move(other.raw_tx);

        hash = std::move(other.hash);

        addr_from = std::move(other.addr_from);
        addr_to = std::move(other.addr_to);

        json_rpc = other.json_rpc;
        other.json_rpc = nullptr;
    }

    return *this;
}

TX::~TX()
{
    delete json_rpc;
}

bool TX::parse(std::string_view raw_data, bool check_sign_flag)
{
    raw_tx.insert(raw_tx.end(), raw_data.begin(), raw_data.end());

    uint64_t index = 0;
    uint8_t varint_size;
    uint64_t sign_data_size = 0;

    {
        const uint8_t toadr_size = 25;
        if (index + toadr_size >= raw_tx.size()) {
            DEBUG_COUT("corrupt addres size");
            return false;
        }
        bin_to = std::string_view(&raw_tx[index], toadr_size);
        index += toadr_size;
    }

    {
        std::string_view varint_arr(&raw_tx[index], raw_tx.size() - index);
        varint_size = crypto::read_varint(value, varint_arr);
        if (varint_size < 1) {
            DEBUG_COUT("corrupt varint size");
            return false;
        }
        index += varint_size;
    }

    {
        std::string_view varint_arr(&raw_tx[index], raw_tx.size() - index);
        varint_size = crypto::read_varint(fee, varint_arr);
        if (varint_size < 1) {
            DEBUG_COUT("corrupt varint size");
            return false;
        }
        index += varint_size;
    }

    {
        std::string_view varint_arr(&raw_tx[index], raw_tx.size() - index);
        varint_size = crypto::read_varint(nonce, varint_arr);
        if (varint_size < 1) {
            DEBUG_COUT("corrupt varint size");
            return false;
        }
        index += varint_size;
    }

    {
        uint64_t data_size;
        std::string_view varint_arr(&raw_tx[index], raw_tx.size() - index);
        varint_size = crypto::read_varint(data_size, varint_arr);
        if (varint_size < 1) {
            DEBUG_COUT("corrupt varint size");
            return false;
        }
        index += varint_size;

        if (index + data_size >= raw_tx.size()) {
            DEBUG_COUT("corrupt data size");
            return false;
        }
        data = std::string_view(&raw_tx[index], data_size);
        index += data_size;

        sign_data_size = index;
    }

    {
        uint64_t sign_size;
        std::string_view varint_arr(&raw_tx[index], raw_tx.size() - index);
        varint_size = crypto::read_varint(sign_size, varint_arr);
        if (varint_size < 1) {
            DEBUG_COUT("corrupt varint size");
            return false;
        }
        index += varint_size;

        if (index + sign_size >= raw_tx.size()) {
            DEBUG_COUT("corrupt sign size");
            return false;
        }
        sign = std::string_view(&raw_tx[index], sign_size);
        index += sign_size;
    }

    {
        uint64_t pubk_size;
        std::string_view varint_arr(&raw_tx[index], raw_tx.size() - index);
        varint_size = crypto::read_varint(pubk_size, varint_arr);
        if (varint_size < 1) {
            DEBUG_COUT("corrupt varint size");
            return false;
        }
        index += varint_size;

        if (pubk_size && (index + pubk_size > raw_tx.size())) {
            DEBUG_COUT("corrupt pub_key size");
            return false;
        }
        pub_key = std::string_view(&raw_tx[index], pubk_size);
        index += pubk_size;
    }

    tx_size = index;

    if (index < raw_tx.size()) {
        std::string_view varint_arr(&raw_tx[index], raw_tx.size() - index);
        varint_size = crypto::read_varint(state, varint_arr);
        if (varint_size < 1) {
            DEBUG_COUT("corrupt varint size - could not read tx state");
        }
    }

    {
        data_for_sign = std::string_view(&raw_tx[0], sign_data_size);
        hash = crypto::get_sha256(raw_tx);
    }

    check_sign_flag = state != TX_STATE_FEE && check_sign_flag;
    if (check_sign_flag && !crypto::check_sign(data_for_sign, sign, pub_key)) {
        DEBUG_COUT("invalid sign");
        return false;
    }

    addr_to = "0x" + crypto::bin2hex(bin_to);
    if (check_sign_flag) {
        auto bin_from = crypto::get_address(pub_key);
        addr_from = "0x" + crypto::bin2hex(bin_from);
    }

    if (state != TX_STATE_APPROVE) {
        if (!data.empty() && data.front() == '{' && data.back() == '}') {
            std::string json_probably(data);
            rapidjson::Document rpc_json;
            if (!rpc_json.Parse(json_probably.c_str()).HasParseError()) {
                if (rpc_json.HasMember("method") && rpc_json["method"].IsString()) {
                    json_rpc = new JSON_RPC();
                    json_rpc->method = std::string(rpc_json["method"].GetString(), rpc_json["method"].GetStringLength());
                    if (rpc_json.HasMember("params") && rpc_json["params"].IsObject()) {
                        const rapidjson::Value& params = rpc_json["params"];
                        for (rapidjson::Value::ConstMemberIterator iter = params.MemberBegin(); iter != params.MemberEnd(); ++iter) {
                            if (iter->name.IsString() && iter->value.IsString()) {
                                json_rpc->parameters[std::string(iter->name.GetString(), iter->name.GetStringLength())] = std::string(iter->value.GetString(), iter->value.GetStringLength());
                            } else {
                                DEBUG_COUT("invalid params");
                                DEBUG_COUT(data);
                            }
                        }
                    }
                }
            }
        }
    }

    return true;
}

bool TX::fill_from_strings(
    std::string& param_to,
    std::string param_value,
    std::string param_fee,
    std::string param_nonce,
    std::string& param_data,
    std::string& param_sign,
    std::string& param_pub_key)
{
    unsigned long transaction_value = 0;
    unsigned long transaction_id = 0;
    unsigned long transaction_fee = 0;

    if (param_value.empty()) {
        param_value = "0";
    }
    try {
        transaction_value = std::stoul(param_value);
    } catch (...) {
        DEBUG_COUT("invalid transaction_value");
        return false;
    }

    if (param_nonce.empty()) {
        param_nonce = "0";
    }
    try {
        transaction_id = std::stoul(param_nonce);
    } catch (...) {
        DEBUG_COUT("invalid transaction_id");
        return false;
    }

    if (param_fee.empty()) {
        param_fee = "0";
    }
    try {
        transaction_fee = std::stoul(param_fee);
    } catch (...) {
        DEBUG_COUT("invalid transaction_fee");
        return false;
    }

    std::vector<unsigned char> bin_to = crypto::hex2bin(param_to);
    std::vector<unsigned char> bin_data = crypto::hex2bin(param_data);
    std::vector<unsigned char> bin_sign = crypto::hex2bin(param_sign);
    std::vector<unsigned char> bin_pub_key = crypto::hex2bin(param_pub_key);

    return fill_sign_n_raw(bin_to, transaction_value, transaction_fee, transaction_id, bin_data, bin_sign, bin_pub_key);
}

void TX::clear()
{
    bin_to = std::string_view();
    value = 0;
    fee = 0;
    nonce = 0;
    data = std::string_view();
    sign = std::string_view();
    pub_key = std::string_view();

    data_for_sign = std::string_view();
    raw_tx.clear();
    hash = { 0 };

    addr_from.clear();
    addr_to.clear();
}

bool TX::check_tx()
{
    if (crypto::check_sign(data_for_sign, sign, pub_key)) {
        hash = crypto::get_sha256(raw_tx);
        addr_to = "0x" + crypto::bin2hex(bin_to);
        auto bin_from = crypto::get_address(pub_key);
        addr_from = "0x" + crypto::bin2hex(bin_from);
        return true;
    }

    DEBUG_COUT("check sign failed");
    return false;
}

}