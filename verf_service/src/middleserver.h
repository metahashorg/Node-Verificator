#ifndef MIDDLESERVER_H
#define MIDDLESERVER_H

#include "MHD.h"
#include "thread_pool.hpp"

struct KeyManager {
    std::vector<char> PrivKey;
    std::vector<char> PubKey;
    std::vector<char> Bin_addr;
    std::string Text_PubKey;
    std::string Text_addres;

    bool parse(const std::string& line);

    std::string make_req_url(std::string& data);

    std::pair<std::string, std::string> sign_string(const std::string& data);
};

class MIDDLE_SERVER : public mh::mhd::MHD {
private:
    std::function<std::string(const std::string&, const std::string&)> processor;

public:
    MIDDLE_SERVER(int _port, std::function<std::string(const std::string&, const std::string&)> func);

    ~MIDDLE_SERVER() override;

    bool run(int thread_number, Request& mhd_req, Response& mhd_resp) override;

protected:
    bool init() override;
};

#endif // MIDDLESERVER_H
