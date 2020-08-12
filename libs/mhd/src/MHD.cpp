#include <arpa/inet.h>
#include <microhttpd.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>
#include <uriparser/Uri.h>
#include "MHD.h"


namespace mh::mhd {

using std::map;
using std::multimap;
using std::string;
using std::vector;

/* MHD static callbacks */
static int grab_key_value(void* cls, enum MHD_ValueKind kind, const char* key, const char* value)
{
    if (cls && key) {
        auto* req = (MHD::Request*)cls;

        if (kind == MHD_HEADER_KIND) {
            if (value)
                req->headers[key] = value;
            else
                req->headers[key];
        }
        else if (kind == MHD_COOKIE_KIND) {
            if (value)
                req->cookies[key] = value;
            else
                req->cookies[key];
        }
        else if (kind == MHD_GET_ARGUMENT_KIND) {
            if (value) {
                req->params[key] = value;
                req->params_multi.insert(std::make_pair(key, value));
            }
            else {
                req->params[key];
                req->params_multi.insert(std::make_pair(key, ""));
            }
        }

        return MHD_YES;
    }

    return MHD_NO;
}

static int mhd_request_callback(void* cls, struct MHD_Connection* connection, const char* url,
                                const char* method, const char* version, const char* upload_data,
                                size_t* upload_data_size, void** ptr)
{
    if (!method
        || (0 != strcmp(method, "GET") && 0 != strcmp(method, "POST")
            && 0 != strcmp(method, "HEAD")))
        return MHD_NO; /* unexpected method */

    auto* req = (MHD::Request*)*ptr;
    if (req == nullptr) {
        /* do never respond on first call */
        req = new MHD::Request;
        req->method.assign(method);
        if (url)
            req->url.assign(url);
        if (version)
            req->version.assign(version);
        if (upload_data && upload_data_size && *upload_data_size) {
            req->post.assign(upload_data, *upload_data_size);
            *upload_data_size = 0;
        }

        MHD_get_connection_values(connection, MHD_GET_ARGUMENT_KIND, grab_key_value, req);
        MHD_get_connection_values(connection, MHD_COOKIE_KIND, grab_key_value, req);
        MHD_get_connection_values(connection, MHD_HEADER_KIND, grab_key_value, req);

        *ptr = req;
        return MHD_YES;
    }

    /* Continue to load post data */
    if (upload_data && upload_data_size && *upload_data_size) {
        req->post.append(upload_data, *upload_data_size);
        *upload_data_size = 0;
        return MHD_YES;
    }


    /* Complete */
    *ptr = nullptr;
    MHD::Response resp;


    if (req->headers.count("SNIPER_TEST")) {
        /* test request */
        resp.code = 204;
        resp.headers["SNIPER_TEST"] = "TEST_OK";
    }
    else {
        MHD* mhd_server = (MHD*)cls;
        mhd_server->run(MHD_get_thread_number(connection), *req, resp);
    }


    delete req;
    int rc = MHD_NO;

    /* Process data */
    struct MHD_Response* response = nullptr;
    if (!resp.data.empty()) {
        response = MHD_create_response_from_buffer(resp.data.size(), (char*)resp.data.c_str(),
                                                   MHD_RESPMEM_MUST_COPY);
    }
    else {
        response = MHD_create_response_from_buffer(0, nullptr, MHD_RESPMEM_PERSISTENT);
    }
    if (response == nullptr)
        return MHD_NO;


    /* Process headers */
    if (!resp.headers.empty()) {
        for (auto& a : resp.headers) {
            rc = MHD_add_response_header(response, a.first.c_str(), a.second.c_str());
            if (rc == MHD_NO)
                return MHD_NO;
        }
    }

    /* Process cookies */
    if (!resp.cookies.empty()) {
        for (auto& a : resp.cookies) {
            if (!a.second.empty()) {
                rc = MHD_add_response_header(response, MHD_HTTP_HEADER_SET_COOKIE,
                                             (a.first + "=" + a.second).c_str());
                if (rc == MHD_NO)
                    return MHD_NO;
            }
        }
    }


    if (resp.code < 200)
        resp.code = 204;

    rc = MHD_queue_response(connection, resp.code, response);
    MHD_destroy_response(response);
    return rc;
}

MHD::MHD() : port(8080), threads_count(1)
{
    /* Linker bug fix
     * libmicrohttpd/src/microhttpd/internal.c:188: undefined reference to `clock_gettime'
     */
    struct timespec tps;
    clock_gettime(CLOCK_REALTIME, &tps);
}

bool MHD::start(const string& path)
{
    this->config_path = path;
    if (this->config_path.empty())
        this->config_path = "./config";
    if (this->config_path.back() != '/')
        this->config_path += "/";

    if (!init())
        return false;

    /* Block signals */
    sigset_t blocked;
    sigemptyset(&blocked);
    sigaddset(&blocked, SIGUSR1);
    sigaddset(&blocked, SIGUSR2);
    pthread_sigmask(SIG_BLOCK, &blocked, nullptr);


    struct MHD_Daemon* d = nullptr;

    if (host.empty()) {
        d = MHD_start_daemon(MHD_USE_EPOLL_INTERNALLY_LINUX_ONLY | MHD_USE_EPOLL_TURBO, port, nullptr,
                             nullptr, &mhd_request_callback, this, MHD_OPTION_THREAD_POOL_SIZE,
                             (unsigned int)threads_count, MHD_OPTION_CONNECTION_LIMIT,
                             (unsigned int)200000, MHD_OPTION_END);
    }
    else {
        struct sockaddr_in sa;
        sa.sin_family = AF_INET;
        sa.sin_port = htons(port);
        if (inet_pton(AF_INET, host.c_str(), &sa.sin_addr) == 1) {
            d = MHD_start_daemon(MHD_USE_EPOLL_INTERNALLY_LINUX_ONLY | MHD_USE_EPOLL_TURBO, port,
                                 nullptr, nullptr, &mhd_request_callback, this,
                                 MHD_OPTION_THREAD_POOL_SIZE, (unsigned int)threads_count,
                                 MHD_OPTION_CONNECTION_LIMIT, (unsigned int)200000,
                                 MHD_OPTION_SOCK_ADDR, (sockaddr*)&sa, MHD_OPTION_END);
        }
    }

    if (!d) {
        fprintf(stderr, "Cannot start MHD daemon\n");
        return false;
    }


    while (true) {
        sigset_t pending;
        sigpending(&pending);

        if (sigismember(&pending, SIGUSR1)) {
            int sig = SIGUSR1;
            sigwait(&pending, &sig);

            usr1();
        }

        if (sigismember(&pending, SIGUSR2)) {
            int sig = SIGUSR2;
            sigwait(&pending, &sig);

            usr2();
        }

        idle();

        sleep(1);
    }


    MHD_stop_daemon(d);

    fini();

    return true;
}

string MHD::set_cookie(const string& key, const string& value, time_t ttl, const string& domain)
{
    if (domain.empty())
        return "";

    char buf[50];
    buf[0] = '\0';

    string exp;

    if (ttl > 0) {
        ttl += time(NULL);
        tm timeinfo;
        gmtime_r(&ttl, &timeinfo);

        if (strftime(buf, 49, "%a, %d %b %Y %H:%M:%S GMT", &timeinfo)) {
            exp = " Expires=";
            exp += buf;
            exp += ";";
        }
    }

    return key + "=" + value + "; Path=/;" + exp + " Domain=" + domain;
}

time_t MHD::get_expiration(enum Expiration exp)
{
    time_t currenttime;
    struct tm local;

    switch (exp) {
        case EXP_NONE:
            return 0;

        case EXP_THISHOUR:
            currenttime = time(nullptr);
            localtime_r(&currenttime, &local);
            local.tm_min = 59;
            local.tm_sec = 59;

            return mktime(&local) - currenttime;

        case EXP_THISDAY:
            currenttime = time(nullptr);
            localtime_r(&currenttime, &local);
            local.tm_hour = 23;
            local.tm_min = 59;
            local.tm_sec = 59;

            return mktime(&local) - currenttime;

        case EXP_THISWEEK:
            currenttime = time(nullptr);
            localtime_r(&currenttime, &local);
            local.tm_hour = 23;
            local.tm_min = 59;
            local.tm_sec = 59;

            return mktime(&local) + 24 * 3600 * (1 + 6 - local.tm_wday) - currenttime;

        case EXP_10MIN:
            return 10 * 60;

        case EXP_THISYEAR:
            currenttime = time(nullptr);
            localtime_r(&currenttime, &local);
            local.tm_mday = 31;
            local.tm_mon = 11; // Month: 0..11
            local.tm_hour = 23;
            local.tm_min = 59;
            local.tm_sec = 59;

            return mktime(&local) - currenttime;

        case EXP_20YEARS:
            return 20 * 365 * 24 * 3600;

        default:
            return 0;
    }
}

void MHD::set_host(const string& host)
{
    this->host = host;
}

void MHD::set_port(unsigned int port)
{
    this->port = port;
}

void MHD::set_threads(unsigned int count)
{
    this->threads_count = count;
}

bool MHD::parse_qs(const string& qs, map<string, string>& params)
{
    if (qs.empty())
        return false;

    params.clear();

    const char* qs_end = strchr(qs.c_str(), '\0');
    if (qs_end && *(qs_end - 1) == '=')
        qs_end--;

    UriQueryListA *query_list;
    int item_count = 0;
    if (uriDissectQueryMallocA(&query_list, &item_count, qs.c_str(), qs_end) != URI_SUCCESS) {
        return false;
    }

    if (!query_list)
        return true;

    int i = 0;
    for (auto* it = query_list; i < item_count && it; it = it->next, i++) {
        if (it->key) {
            if (it->value)
                params.insert(std::pair<string, string>(it->key, it->value));
            else
                params.insert(std::pair<string, string>(it->key, ""));
        }
    }

    uriFreeQueryListA(query_list);

    return true;
}

const string& MHD::get_config_path()
{
    return config_path;
}

unsigned int MHD::get_threads()
{
    return threads_count;
}

} // namespace mh::mhd
