#include <mhcurl.hpp>

size_t curlwritefunc(void* ptr, size_t size, size_t nmemb, std::string* p_resp)
{
    std::string& resp = *p_resp;
    resp.insert(resp.end(), reinterpret_cast<char*>(ptr), reinterpret_cast<char*>(ptr) + nmemb);
    return size * nmemb;
}

bool CurlFetch::post(const std::string& url, const std::string& reques_string, std::string& response)
{
    if (!curl) {
        curl = curl_easy_init();
    }

    std::string path;
    if (!url.empty() && url[0] == '/') {
        path.insert(path.end(), url.begin() + 1, url.end());
    } else {
        path = url;
    }

    std::string full_url = "http://" + host + "/" + url;
    curl_easy_setopt(curl, CURLOPT_URL, full_url.c_str());
    curl_easy_setopt(curl, CURLOPT_PORT, port);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, reques_string.size());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, reques_string.data());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlwritefunc);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
    int ret = curl_easy_perform(curl);

    if (!ret) {
        return true;
    }

    curl_easy_cleanup(curl);
    curl = nullptr;

    return false;
}

bool CurlFetch::post_singned(
    const std::string& url,
    const std::string& reques_string,
    const std::string& sign,
    const std::string& bubk,
    std::string& response)
{
    if (!curl) {
        curl = curl_easy_init();
    }

    std::string path;
    if (!url.empty() && url[0] == '/') {
        path.insert(path.end(), url.begin() + 1, url.end());
    } else {
        path = url;
    }

    struct curl_slist* chunk = NULL;
    std::string sign_header = "Sign: " + sign;
    std::string pubk_header = "PublicKey: " + bubk;
    chunk = curl_slist_append(chunk, sign_header.c_str());
    chunk = curl_slist_append(chunk, pubk_header.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

    std::string full_url = "http://" + host + "/" + path;
    curl_easy_setopt(curl, CURLOPT_URL, full_url.c_str());

    curl_easy_setopt(curl, CURLOPT_PORT, port);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, reques_string.size());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, reques_string.data());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlwritefunc);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    int ret = curl_easy_perform(curl);

    curl_slist_free_all(chunk);

    if (!ret) {
        return true;
    }

    curl_easy_cleanup(curl);
    curl = nullptr;

    return false;
}
