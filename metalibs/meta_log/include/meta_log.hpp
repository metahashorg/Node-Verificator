#ifndef META_LOG_HPP
#define META_LOG_HPP

#define DEBUG_OUTPUT

#ifdef DEBUG_OUTPUT
#include <concurrentqueue.h>
#include <sstream>
#include <thread>

extern moodycamel::ConcurrentQueue<std::stringstream*>* __output_queue;
extern std::thread* __cout_printer;

void print_to_stream_date_and_place(std::stringstream * p_ss, const std::string& file, const std::string& function, int line);

#endif

#define DEBUG_COUT(message) _debug_cout(true, std::string(__FILE__), std::string(&__func__[0]), __LINE__, message)
#define DEBUG_COUT_COND(print, message) _debug_cout(print, std::string(__FILE__), std::string(&__func__[0]), __LINE__, message)

template <typename Message>
inline void _debug_cout(bool print, const std::string& file, const std::string& function, const int line, const Message& msg)
{
    if (print) {
#ifdef DEBUG_OUTPUT

        auto* p_ssout = new std::stringstream;

        print_to_stream_date_and_place(p_ssout, file, function, line);

        (*p_ssout) << msg;

        __output_queue->enqueue(p_ssout);
#endif
    }
}

#endif // META_LOG_HPP
