#ifndef REST_CLIENT_HPP
#define REST_CLIENT_HPP

#include <string>

std::string place_order(const std::string& access_token,
                        const std::string& instrument,
                        double amount,
                        const std::string& side,
                        const std::string& order_type,
                        double price = 0.0);

std::string cancel_order(const std::string& access_token, const std::string& order_id);

std::string modify_order(const std::string& access_token, const std::string& order_id, double new_amount, double new_price);

std::string get_order_book(const std::string& instrument, int depth = 10);

std::string get_positions(const std::string& access_token, const std::string& currency);

#endif
