#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>

std::string http_get(const std::string& url);

std::string http_post(const std::string& url, const std::string& post_fields, const std::string& auth_token = "");

std::string http_get_with_auth(const std::string& url, const std::string& auth_token);
#endif