#include "middleserver.h"
#include "open_ssl_decor.h"
#include "statics.hpp"
#include <meta_log.hpp>
#include <transaction.h>

MIDDLE_SERVER::MIDDLE_SERVER(
    int _port,
    std::map<std::string, moodycamel::ConcurrentQueue<std::string*>>& _send_message_queue,
    uint64_t _pool_size,
    std::atomic<int64_t>& _counter)
    : send_message_queue(_send_message_queue)
    , pool_size(_pool_size)
    , counter(_counter)
{
    set_port(_port);
    set_threads(std::thread::hardware_concurrency());
}

MIDDLE_SERVER::~MIDDLE_SERVER() = default;

bool MIDDLE_SERVER::run(int /*thread_number*/, mh::mhd::MHD::Request& mhd_req, mh::mhd::MHD::Response& mhd_resp)
{
    counter++;

    // if (send_message_queue.begin()->second.size_approx() > pool_size) {
    //     mhd_resp.data = "Queue full<BR/>";
    //     return true;
    // }

    TX* p_tx = new TX;
    if (!p_tx->parse(mhd_req.post)) {
        if (!mhd_req.post.empty()) {
            DEBUG_COUT(bin2hex(mhd_req.post));
        }
        for (const auto& pair : mhd_req.headers) {
            DEBUG_COUT(pair.first + "\t" + pair.second);
        }
        delete p_tx;
        mhd_resp.data = "Transaction corruped.<BR/>";
        return false;
    }

    delete p_tx;

    for (auto& network_queue : send_message_queue) {
        auto* req_post = new std::string(mhd_req.post);
        network_queue.second.enqueue(req_post);
    }
    mhd_resp.data = "Transaction accepted.<BR/>";
    return true;
}

bool MIDDLE_SERVER::init()
{
    return true;
}