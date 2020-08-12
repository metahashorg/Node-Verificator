#include "middleserver.h"

#include <meta_log.hpp>
#include <meta_constants.hpp>
#include <thread>

MIDDLE_SERVER::MIDDLE_SERVER(int _port, std::function<std::string(const std::string&, const std::string&)> func)
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

std::vector<std::string> split(const std::string& s, char delim)
{
    std::stringstream ss(s);
    std::string item;
    std::vector<std::string> elems;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

std::set<std::tuple<std::string, std::string, int>> parse_core_list(const std::vector<char>& data)
{
    std::string in_string(data.data(), data.size());
    std::set<std::tuple<std::string, std::string, int>> core_list;

    auto records = split(in_string, '\n');
    for (const auto& record : records) {
        auto host_port = split(record, ':');
        if (host_port.size() == 3) {
            core_list.insert({ host_port[0], host_port[1], std::stoi(host_port[2]) });
        }
    }

    return core_list;
}

CoreConnector::CoreConnector(boost::asio::io_context& io_context, metahash::crypto::Signer& signer)
    : io_context(io_context)
    , signer(signer)
{
}

void CoreConnector::init(const std::map<std::string, std::pair<std::string, int>>& core_list)
{
    for (auto&& [mh_addr, host_port_pair] : core_list) {
        auto&& [host, port] = host_port_pair;
        cores.emplace(mh_addr, new metahash::network::meta_client(io_context, mh_addr, host, port, concurrent_connections_count, signer));
    }
}

void CoreConnector::sync_core_lists()
{
        auto resp = send_with_return(RPC_GET_CORE_LIST, std::vector<char>());

        for (auto&& [mh_addr, data] : resp) {
            {
                std::string data_str;
                data_str.insert(data_str.end(), data.begin(), data.end());
                DEBUG_COUT(mh_addr);
                DEBUG_COUT(data_str);
            }

            auto hosts = parse_core_list(data);

            add_new_cores(hosts);
        }
}

void CoreConnector::add_new_cores(const std::set<std::tuple<std::string, std::string, int>>& hosts)
{
    std::lock_guard lock(core_lock);
    for (auto&& [addr, host, port] : hosts) {
        if (addr != signer.get_mh_addr()) {
            if (cores.find(addr) == cores.end()) {
                cores.emplace(addr, new metahash::network::meta_client(io_context, addr, host, port, concurrent_connections_count, signer));
            }
        }
    }
}

void CoreConnector::send_no_return(uint64_t req_type, const std::vector<char>& req_data)
{
    std::lock_guard lock(core_lock);
    for (auto&& [mh_addr, core] : cores) {
        core->send_message(req_type, req_data, [](const std::vector<char>&) {});
    }
}

std::map<std::string, std::vector<char>> CoreConnector::send_with_return(uint64_t req_type, const std::vector<char>& req_data)
{
    std::map<std::string, std::vector<char>> resp_strings;
    std::map<std::string, std::future<std::vector<char>>> futures;

    {
        std::lock_guard lock(core_lock);
        for (auto&& [mh_addr, core] : cores) {
            auto promise = std::make_shared<std::promise<std::vector<char>>>();
            futures.insert({ mh_addr, promise->get_future() });

            core->send_message(req_type, req_data, [promise](const std::vector<char>& resp) {
                promise->set_value(resp);
            });
        }
    }

    for (auto&& [mh_addr, future] : futures) {
        auto data = future.get();

        resp_strings[mh_addr] = data;
    }

    return resp_strings;
}
