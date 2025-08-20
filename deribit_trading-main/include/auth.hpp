#ifndef AUTH_HPP
#define AUTH_HPP

#include <string>

std::string get_access_token(const std::string& client_id, const std::string& client_secret);

#endif
