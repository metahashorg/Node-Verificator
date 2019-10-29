#include "middleserver.h"

#include <meta_log.hpp>
#include <open_ssl_decor.h>
#include <statics.hpp>
#include <utility>

MIDDLE_SERVER::MIDDLE_SERVER(int _port, std::function<std::string(const std::string&, const std::string&)>  func)
    : processor(std::move(func))
{
    set_port(_port);
    set_threads(std::thread::hardware_concurrency());
}

MIDDLE_SERVER::~MIDDLE_SERVER() = default;

bool MIDDLE_SERVER::run(int /*thread_number*/, mh::mhd::MHD::Request& mhd_req, mh::mhd::MHD::Response& mhd_resp)
{
    std::string resp = processor(mhd_req.post, mhd_req.url);

    if (resp.empty()) {
        mhd_resp.data = "ok";
    } else {
        mhd_resp.data = std::move(resp);
    }
    return true;
}

bool MIDDLE_SERVER::init()
{
    return true;
}

bool KeyManager::parse(const std::string& line)
{

    std::vector<unsigned char> priv_k = hex2bin(line);
    PrivKey.insert(PrivKey.end(), priv_k.begin(), priv_k.end());
    if (!generate_public_key(PubKey, PrivKey)) {
        return false;
    }

    Text_PubKey = "0x" + bin2hex(PubKey);

    std::array<char, 25> addres = get_address(PubKey);
    Bin_addr.insert(Bin_addr.end(), addres.begin(), addres.end());

    Text_addres = "0x" + bin2hex(Bin_addr);

    return true;
}

std::string KeyManager::make_req_url(std::string& data)
{
    std::vector<char> sign;
    sign_data(data, sign, PrivKey);

    return "/?pubk=" + Text_PubKey + "&sign=" + bin2hex(sign);
}