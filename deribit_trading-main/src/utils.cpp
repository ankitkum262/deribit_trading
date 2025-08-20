#include "utils.hpp"
#include <curl/curl.h>
#include <stdexcept>

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t total_size = size * nmemb;
    output->append((char*)contents, total_size);
    return total_size;
}

std::string http_get(const std::string& url) {
    CURL* curl = curl_easy_init();
    if (!curl) throw std::runtime_error("CURL init failed");

    std::string response;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if(res != CURLE_OK) throw std::runtime_error("CURL GET error");
    return response;
}

std::string http_post(const std::string& url, const std::string& post_fields, const std::string& auth_token) {
    CURL* curl = curl_easy_init();
    if (!curl) throw std::runtime_error("CURL init failed");

    struct curl_slist *headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    if (!auth_token.empty()) {
        std::string auth_header = "Authorization: Bearer " + auth_token;
        headers = curl_slist_append(headers, auth_header.c_str());
    }

    std::string response;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    if (!post_fields.empty()) {
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_fields.c_str());
    }
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if(res != CURLE_OK)
        throw std::runtime_error("CURL POST error: " + std::string(curl_easy_strerror(res)));

    return response;
}

std::string http_get_with_auth(const std::string& url, const std::string& auth_token) {
    CURL* curl = curl_easy_init();
    if (!curl) throw std::runtime_error("CURL init failed");

    struct curl_slist *headers = nullptr;
    if (!auth_token.empty()) {
        std::string auth_header = "Authorization: Bearer " + auth_token;
        headers = curl_slist_append(headers, auth_header.c_str());
    }

    std::string response;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if(res != CURLE_OK)
        throw std::runtime_error("CURL GET error: " + std::string(curl_easy_strerror(res)));

    return response;
}


