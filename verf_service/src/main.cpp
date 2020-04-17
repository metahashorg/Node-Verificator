#include <set>
#include <unordered_map>

#include <fstream>
#include <iostream>
#include <sstream>

#include <meta_log.hpp>
#include <mhcurl.hpp>
#include <open_ssl_decor.h>
#include <statics.hpp>
#include <transaction.h>

#include "middleserver.h"

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <help_lib.hpp>
#include <version.h>

void sender_func(std::map<std::string, moodycamel::ConcurrentQueue<std::string*>>& send_message_map, KeyManager& key_holder, std::string network, std::string host, int port)
{
    CurlFetch CF(host, port);
    std::string* p_req_post[1024];
    moodycamel::ConcurrentQueue<std::string*>& send_message = send_message_map[network];
    uint64_t i = 0;

    bool do_it_forever = true;
    while (do_it_forever) {
        if (uint got_msg = send_message.try_dequeue_bulk(p_req_post, 1024)) {
            std::string req;

            for (uint i = 0; i < got_msg; i++) {
                append_varint(req, p_req_post[i]->size());
                req.insert(req.end(), p_req_post[i]->begin(), p_req_post[i]->end());
                delete p_req_post[i];
            }
            append_varint(req, 0);

            auto&& [sign, pubk] = key_holder.sign_string(req);

            std::string path = "/" + RPC_TX + "/";
            std::string response;
            while (true) {
                if (CF.post_singned(path, req, sign, pubk, response)) {
                    break;
                }
                if (i % 1000 == 0) {
                    DEBUG_COUT("Curl request not sent");
                }
                i++;
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
}

void SIGPIPE_handler(int /*s*/)
{
    DEBUG_COUT("Caught SIGPIPE");
}

[[noreturn]] void SIGSEGV_handler(int /*s*/)
{
    DEBUG_COUT("Caught SIGSEGV");
    std::this_thread::sleep_for(std::chrono::seconds(2));
    exit(1);
}

[[noreturn]] void SIGTERM_handler(int /*s*/)
{
    DEBUG_COUT("Caught SIGTERM");
    std::this_thread::sleep_for(std::chrono::seconds(2));
    exit(0);
}

int main(int argc, char** argv)
{
    std::map<std::string, moodycamel::ConcurrentQueue<std::string*>> send_message_map;

    DEBUG_COUT("Version:\t" + std::string(VESION_MAJOR) + "." + std::string(VESION_MINOR) + "." + std::string(GIT_COMMIT_HASH));

    int listen_port = 0;
    std::string network;
    KeyManager key_holder;
    std::vector<std::thread> sender;

    if (argc > 1) {
        std::ifstream ifs(argv[1]);
        std::string content((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));

        rapidjson::Document config_json;
        if (!config_json.Parse(content.c_str()).HasParseError()) {

            if (config_json.HasMember("network") && config_json["network"].IsString()
                && config_json.HasMember("key") && config_json["key"].IsString()
                && config_json.HasMember("port") && config_json["port"].IsUint()
                && config_json.HasMember("cores") && config_json["cores"].IsArray()) {

                listen_port = config_json["port"].GetUint();
                if (listen_port == 0) {
                    std::cerr << "Errors in configuration file" << std::endl;
                    std::cerr << "Invalid port" << std::endl;
                    exit(1);
                }

                network = config_json["network"].GetString();

                if (!key_holder.parse(std::string(config_json["key"].GetString()))) {
                    std::cerr << "Error while parsing Private key" << std::endl;
                    exit(1);
                }

                std::map<std::string, std::pair<std::string, int>> cores;
                auto& v_list = config_json["cores"];

                for (uint i = 0; i < v_list.Size(); i++) {
                    auto& record = v_list[i];

                    if (record.HasMember("network") && record["network"].IsString()
                        && record.HasMember("host") && record["host"].IsString()
                        && record.HasMember("port") && record["port"].IsUint()
                        && record["port"].GetUint() != 0) {

                        cores.insert({ record["network"].GetString(), { record["host"].GetString(), record["port"].GetUint() } });
                    }
                }

                if (cores.empty()) {
                    std::cerr << "Errors in configuration file" << std::endl;
                    std::cerr << "Missing or invalid parameters" << std::endl;
                    exit(1);
                }

                for (const auto& core : cores) {
                    send_message_map[core.first];
                    uint i_max = 2;
                    if (core.first == "net-main") {
                        i_max = 4;
                    }
                    for (uint i = 0; i < i_max; i++) {
                        sender.emplace_back(sender_func, std::ref(send_message_map), std::ref(key_holder), core.first, core.second.first, core.second.second);
                    }
                }

            } else {
                std::cerr << "Errors in configuration file" << std::endl;
                std::cerr << "Missing or invalid parameters" << std::endl;
                exit(1);
            }
        } else {
            std::cerr << "Ivalid configuration file" << std::endl;
            exit(1);
        }
    } else {
        std::cerr << "Usage:" << std::endl;
        std::cerr << "app [config file]" << std::endl;
        exit(1);
    }

    std::atomic<int64_t> counter_(0);
    std::thread trd([&counter_, &send_message_map, network]() {
        CurlFetch CF("172.104.236.166", 5797);

        const std::string ip = getMyIp();
        const std::string host = getHostName();
        const std::string version = std::string(VESION_MAJOR) + "." + std::string(VESION_MINOR) + "." + std::string(GIT_COMMIT_HASH);
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(1));

            uint64_t timestamp = std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::system_clock::now()).time_since_epoch().count();
            std::string req_post;

            {
                rapidjson::StringBuffer s;
                rapidjson::Writer<rapidjson::StringBuffer> writer(s);

                auto write_string_metric = [&writer](std::string key, std::string value) {
                    writer.StartObject();
                    {

                        writer.String("metric");
                        writer.String(key.c_str());

                        writer.String("type");
                        writer.String("none");

                        writer.String("value");
                        writer.String(value.c_str());
                    }
                    writer.EndObject();
                };

                auto write_sum_metric = [&writer](std::string key, uint64_t value) {
                    writer.StartObject();
                    {

                        writer.String("metric");
                        writer.String(key.c_str());

                        writer.String("type");
                        writer.String("sum");

                        writer.String("value");
                        writer.Uint64(value);
                    }
                    writer.EndObject();
                };

                writer.StartObject();
                {

                    writer.String("params");
                    writer.StartObject();
                    {

                        writer.String("network");
                        writer.String(network.c_str());

                        writer.String("group");
                        writer.String("verif");

                        writer.String("server");
                        writer.String(host.c_str());

                        writer.String("timestamp_ms");
                        writer.Uint64(timestamp);

                        writer.String("metrics");
                        writer.StartArray();
                        {
                            write_sum_metric("qps", counter_.load());
                            write_sum_metric("queue_size", send_message_map[network].size_approx());

                            write_string_metric("ip", ip);
                            write_string_metric("version", version);
                        }
                        writer.EndArray();
                    }
                    writer.EndObject();
                }
                writer.EndObject();

                req_post = std::string(s.GetString());
            }

            std::string response;
            CF.post("save-metrics", req_post, response);

            counter_.store(0);
        }
    });

    MIDDLE_SERVER MS(listen_port, [&counter_, &send_message_map, &key_holder](const std::string& req_post, const std::string& req_url) {
        counter_++;

        // if (send_message_queue.begin()->second.size_approx() > pool_size) {
        //     mhd_resp.data = "Queue full<BR/>";
        //     return true;
        // }

        std::string path = req_url;
        path.erase(std::remove(path.begin(), path.end(), '/'), path.end());

        if (path == "getinfo") {
            static const std::string version = std::string(VESION_MAJOR) + "." + std::string(VESION_MINOR) + "." + std::string(GIT_COMMIT_HASH);
            rapidjson::StringBuffer s;
            rapidjson::Writer<rapidjson::StringBuffer> writer(s);
            writer.StartObject();
            {
                writer.String("result");
                writer.StartObject();
                {
                    writer.String("version");
                    writer.String(version.c_str());
                    writer.String("mh_addr");
                    writer.String(key_holder.Text_addres.c_str());
                }
                writer.EndObject();
            }
            writer.EndObject();
            DEBUG_COUT("Version");
            return std::string(s.GetString());
        } else {
            TX* p_tx = new TX;
            if (!p_tx->parse(req_post)) {
                DEBUG_COUT("Transaction corrupted");

                if (!req_post.empty()) {
                    DEBUG_COUT(bin2hex(req_post));
                }

                delete p_tx;
                return std::string("Transaction corrupted.<BR/>");
            }

            delete p_tx;

            for (auto& network_queue : send_message_map) {
                auto* p_string_post = new std::string(req_post);
                network_queue.second.enqueue(p_string_post);
            }
            DEBUG_COUT("Transaction accepted");
            return std::string("Transaction accepted.<BR/>");
        }
    });

    MS.start();

    return 0;
}
