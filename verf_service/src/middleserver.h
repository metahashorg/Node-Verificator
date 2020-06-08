#ifndef MIDDLESERVER_H
#define MIDDLESERVER_H

#include "MHD.h"

#include <functional>
#include <thread>

#include <meta_client.h>
#include <open_ssl_decor.h>
#include <thread_pool.hpp>

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

class CoreConnector {
private:
    const uint64_t concurrent_connections_count = 4;

    std::mutex core_lock;
    std::map<std::string, metahash::net_io::meta_client*> cores;

    boost::asio::io_context& io_context;

    metahash::crypto::Signer& signer;

public:
    CoreConnector(boost::asio::io_context& io_context, metahash::crypto::Signer&);

    void init(const std::map<std::string, std::pair<std::string, int>>& core_list);

    void sync_core_lists();
    void add_new_cores(const std::set<std::tuple<std::string, std::string, int>>& hosts);

    void add_cores(std::string_view pack);
    std::vector<char> get_core_list();

    void send_no_return(uint64_t req_type, const std::vector<char>& req_data);
    std::map<std::string, std::vector<char>> send_with_return(uint64_t req_type, const std::vector<char>& req_data);

    void send_no_return_to_core(const std::string& addr, uint64_t req_type, const std::vector<char>& req_data);
    std::vector<char> send_with_return_to_core(const std::string& addr, uint64_t req_type, const std::vector<char>& req_data);
};

#endif // MIDDLESERVER_H
