#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <unistd.h>
#include <sys/wait.h>
#include "nlohmann/json.hpp"
#include "../include/db.h"
using json = nlohmann::json;


std::string PATH;
void set_path(const std::string& p) {
    PATH = p;                 // <-- assign to the global PATH
}

json load_bssid_database() {
    std::ifstream f(PATH);
    if (!f.is_open()) {
        std::cerr << "Failed to open file: " << PATH << std::endl;
        return json::array(); // safe default
    }

    try {
        json j = json::parse(f);
        if (!j.is_array()) {
            std::cerr << "Database is not an array, resetting.\n";
            return json::array();
        }
        return j;
    } catch (const json::parse_error& e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        return json::array(); // safe default
    }
}

void save_bssid_database(const json& j) {
    std::ofstream f(PATH);
    f << j.dump(4);
}

bool exists_in_db(const std::string& bssid) {
    json j = load_bssid_database();
    bool eidb = false;
    for (const auto& entry : j) {
        if (entry.contains("bssid") && entry["bssid"] == bssid) {
            eidb = true;
            break;
        }
    }
    return eidb;
}

void add_entry(
    const std::string& bssid,
    const int channel
) {
    json j = load_bssid_database();
    if (!j.is_array()){ 
        j=json::array();
    }
    j.push_back({
        {"bssid", bssid},
        {"channel", channel}
    });

    save_bssid_database(j);
}
void remove_entry(const std::string& bssid) {
    json j = load_bssid_database();
    if (!j.is_array()) {
        std::cerr << "Database is not an array, cannot remove entry.\n";
        return;
    }
    json new_db = json::array();
    for (const auto& entry : j) {
        if (entry.contains("bssid") && entry["bssid"] != bssid) {
            new_db.push_back(entry);
        }
    }
    save_bssid_database(new_db);
} 
std::vector<Network> load_any_database(const std::string& path) {

    std::vector<Network> networks;

    std::ifstream file(path);

    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file");
    }

    try {

        json j;
        file >> j;

        if (!j.is_array()) {
            throw std::runtime_error("JSON is not array");
        }

        for (const auto& item : j) {

            Network n;

            n.bssid =
                item["bssid"].get<std::string>();

            n.channel =
                item["channel"].get<int>();

            networks.push_back(n);
        }

        std::cout << "Loaded JSON database\n";

        return networks;

    } catch (...) {

        // Reset stream state
        file.clear();
        file.seekg(0);

        std::cout << "Not JSON, trying TXT format\n";
    }

    // -------------------------
    // TXT parser
    // -------------------------
    std::string line;

    while (std::getline(file, line)) {

        if (line.empty()) {
            continue;
        }

        std::stringstream ss(line);

        Network n;

        ss >> n.bssid >> n.channel;

        if (ss.fail()) {
            std::cerr << "Invalid line: "
                      << line << std::endl;
            continue;
        }

        networks.push_back(n);
    }

    return networks;
}