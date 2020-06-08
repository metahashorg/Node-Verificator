#include <set>
#include <unordered_map>

#include <fstream>
#include <iostream>

#include <meta_log.hpp>
#include <open_ssl_decor.h>
#include <statics.hpp>
#include <transaction.h>

#include "middleserver.h"

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <version.h>

void get_cores(const boost::system::error_code&, boost::asio::deadline_timer& t, CoreConnector& cores)
{
    cores.sync_core_lists();
    t.expires_at(t.expires_at() + boost::posix_time::minutes(5));
    t.async_wait(boost::bind(get_cores, boost::asio::placeholders::error, std::ref(t), std::ref(cores)));
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

    DEBUG_COUT("Version:\t" + std::string(VESION_MAJOR) + "." + std::string(VESION_MINOR) + "." + std::string(GIT_COUNT));

    int listen_port = 0;
    std::string network;
    std::string hex_priv_key;
    std::map<std::string, std::pair<std::string, int>> core_list;

    if (argc > 1) {
        std::ifstream ifs(argv[1]);
        std::string content((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));

        rapidjson::Document config_json;
        if (!config_json.Parse(content.c_str()).HasParseError()) {

            if (config_json.HasMember("network") && config_json["network"].IsString()
                && config_json.HasMember("key") && config_json["key"].IsString()
                && config_json.HasMember("port") && config_json["port"].IsUint()
                && config_json.HasMember("cores") && config_json["cores"].IsArray()) {

                listen_port = config_json["port"].GetInt();
                if (listen_port == 0) {
                    std::cerr << "Errors in configuration file" << std::endl;
                    std::cerr << "Invalid port" << std::endl;
                    exit(1);
                }

                network = config_json["network"].GetString();
                hex_priv_key = config_json["key"].GetString();

                auto& v_list = config_json["cores"];

                for (uint i = 0; i < v_list.Size(); i++) {
                    auto& record = v_list[i];

                    if (record.HasMember("network") && record["network"].IsString()
                        && record.HasMember("host") && record["host"].IsString()
                        && record.HasMember("port") && record["port"].IsUint()
                        && record["port"].GetUint() != 0) {

                        core_list.insert({ record["network"].GetString(), { record["host"].GetString(), record["port"].GetUint() } });
                    }
                }

                if (core_list.empty()) {
                    std::cerr << "Errors in configuration file" << std::endl;
                    std::cerr << "Missing or invalid parameters" << std::endl;
                    exit(1);
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
    boost::asio::io_context io_context;
    metahash::crypto::Signer signer(metahash::crypto::hex2bin(hex_priv_key));
    CoreConnector cores(io_context, signer);

    MIDDLE_SERVER MS(listen_port, [&cores, &signer](const std::string& req_post, const std::string& req_url) {
        std::string path = req_url;
        path.erase(std::remove(path.begin(), path.end(), '/'), path.end());

        if (path == "getinfo") {
            static const std::string version = std::string(VESION_MAJOR) + "." + std::string(VESION_MINOR) + "." + std::string(GIT_COUNT);
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
                    writer.String(signer.get_mh_addr().c_str());
                }
                writer.EndObject();
            }
            writer.EndObject();
            return std::string(s.GetString());
        } else {
            {
                auto* p_tx = new metahash::metachain::TX();
                if (!p_tx->parse(req_post)) {
                    DEBUG_COUT("Transaction corrupted");

                    if (!req_post.empty()) {
                        DEBUG_COUT(metahash::crypto::bin2hex(req_post));
                    }

                    delete p_tx;
                    return std::string("Transaction corrupted.<BR/>");
                }

                delete p_tx;
            }


            {
                std::vector<char> data;
                data.insert(data.end(), req_post.begin(), req_post.end());
                cores.send_no_return(RPC_TX, data);
            }

            return std::string("Transaction accepted.<BR/>");
        }
    });

    auto&& [threads, work] = thread_pool(io_context, std::thread::hardware_concurrency());

    boost::asio::deadline_timer t(io_context, boost::posix_time::minutes(5));
    t.async_wait(boost::bind(get_cores, boost::asio::placeholders::error, std::ref(t), std::ref(cores)));

    cores.init(core_list);
    MS.start();

    return 0;
}
