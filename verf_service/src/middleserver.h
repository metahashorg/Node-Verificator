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
};

class MIDDLE_SERVER : public mh::mhd::MHD {
private:
    std::map<std::string, moodycamel::ConcurrentQueue<std::string*>>& send_message_queue;
    uint64_t pool_size;
    std::atomic<int64_t>& counter;

public:
    MIDDLE_SERVER(int _port, std::map<std::string, moodycamel::ConcurrentQueue<std::string*>>& _send_message_queue, uint64_t _pool_size, std::atomic<int64_t>& _counter);

    virtual ~MIDDLE_SERVER();

    virtual bool run(int thread_number, Request& mhd_req, Response& mhd_resp);

protected:
    virtual bool init();
};

#endif // MIDDLESERVER_H
