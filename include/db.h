#pragma once
#include <nlohmann/json.hpp>
using json = nlohmann::json;
#include <string>
#include <fstream>
void set_path(const std::string& p);
json load_bssid_database();
void save_bssid_database(const json& j);
bool exists_in_db(const std::string& bssid);
void add_entry(
    const std::string& bssid,
    const int channel
);
void remove_entry(const std::string& bssid);
struct Network {
    std::string bssid;
    int channel;
};
std::vector<Network> load_any_database(const std::string& path);