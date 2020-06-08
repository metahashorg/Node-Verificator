#include "controller.hpp"
#include <meta_log.hpp>
#include <open_ssl_decor.h>

namespace metahash::metachain {

bool ApproveRecord::make(
    const sha256_2& approving_block_hash,
    crypto::Signer & signer)
{
    std::vector<char> PubKey = signer.get_pub_key();
    std::vector<char> sign_buff = signer.sign(approving_block_hash);

    std::vector<char> record_raw;

    record_raw.insert(record_raw.end(), approving_block_hash.begin(), approving_block_hash.end());

    crypto::append_varint(record_raw, sign_buff.size());
    record_raw.insert(record_raw.end(), sign_buff.begin(), sign_buff.end());

    crypto::append_varint(record_raw, PubKey.size());
    record_raw.insert(record_raw.end(), PubKey.begin(), PubKey.end());

    std::string_view record_raw_sw(record_raw.data(), record_raw.size());
    return parse(record_raw_sw);
}

bool ApproveRecord::parse(std::string_view ap_sw)
{
    uint64_t index = 0;
    uint64_t sign_varint_size;
    uint64_t sign_size;
    uint64_t pubk_varint_size;
    uint64_t pubk_size;
    {
        uint64_t hash_size = 32;

        if (index + hash_size >= ap_sw.size()) {
            DEBUG_COUT("corrupt data size");
            return false;
        }
        block_hash = std::string_view(&ap_sw[index], hash_size);
        index += hash_size;
    }

    {
        std::string_view varint_arr(&ap_sw[index], ap_sw.size() - index);
        sign_varint_size = crypto::read_varint(sign_size, varint_arr);
        if (sign_varint_size < 1) {
            DEBUG_COUT("corrupt varint size");
            return false;
        }
        index += sign_varint_size;

        if (sign_size && (index + sign_size >= ap_sw.size())) {
            DEBUG_COUT("corrupt sign size");
            return false;
        }
        sign = std::string_view(&ap_sw[index], sign_size);
        index += sign_size;
    }

    {
        std::string_view varint_arr(&ap_sw[index], ap_sw.size() - index);
        pubk_varint_size = crypto::read_varint(pubk_size, varint_arr);
        if (pubk_varint_size < 1) {
            DEBUG_COUT("corrupt varint size");
            return false;
        }
        index += pubk_varint_size;

        if (pubk_size && (index + pubk_size > ap_sw.size())) {
            DEBUG_COUT("corrupt pub_key size");
            return false;
        }
        pub_key = std::string_view(&ap_sw[index], pubk_size);
        index += pubk_size;
    }

    if (!crypto::check_sign(block_hash, sign, pub_key)) {
        DEBUG_COUT("invalid sign");
        return false;
    }

    data.insert(data.begin(), &ap_sw[0], &ap_sw[index]);
    {
        index = 0;
        block_hash = std::string_view(&data[index], 32);
        index += 32;
        index += sign_varint_size;
        sign = std::string_view(&data[index], sign_size);
        index += sign_size;
        index += pubk_varint_size;
        pub_key = std::string_view(&data[index], pubk_size);
    }

    return true;
}

}