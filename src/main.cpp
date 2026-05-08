#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <unistd.h>
#include <sys/wait.h>
#include "nlohmann/json.hpp"
#include "../include/db.h"
#include <atomic>
#include <vector>
#include <thread>
using json = nlohmann::json;

int main(int argc, char* argv[]){
    if (argc < 3){
        std::cerr <<"Every function here has more than 1 argument." << std::endl;
        return 1;
    }
    std::string operation = argv[1];
    if (operation == "add"){
        if (argc < 5){
            std::cerr <<"add operation requires 3 arguments: bssid, channel and path to database file." << std::endl;
            return 1;
        }
        std::string bssid = argv[2];
        int channel = std::stoi(argv[3]);
        std::string path = argv[4];
        set_path(path);
        if (!exists_in_db(bssid)){
            std::cout << "Adding entry: " << bssid << " on channel " << channel << std::endl;
            add_entry(bssid, channel);
        } else {
            std::cout << "Entry already exists: " << bssid << std::endl;
        }
    } else if (operation == "remove"){
        if (argc < 4){
            std::cerr <<"remove operation requires 2 arguments: bssid and path to database file." << std::endl;
            return 1;
        }
        std::string bssid = argv[2];
        std::string path = argv[3];
        set_path(path);
        json j = load_bssid_database();
        remove_entry(bssid);
        std::cout << "Removed entry: " << bssid << std::endl;
    } else if (operation == "takedown") {

    if (geteuid() != 0) {
        std::cerr << "This program must be run as root!\n";
        return 1;
    }

    if (argc < 5) {
        std::cerr << "takedown requires: packet_number interface db\n";
        return 1;
    }

    std::string packet_number = argv[2];
    std::string interface = argv[3];
    std::string path = argv[4];

    set_path(path);

    json j = load_bssid_database();

    if (!j.is_array() || j.empty()) {
        std::cerr << "Database empty or invalid\n";
        return 1;
    }

    // ----------------------------
    // Build job list
    // ----------------------------
    std::vector<json> jobs = j;

    const int WORKERS = 4; // adjust: 2–8 is typical

    std::atomic<size_t> index = 0;

    auto worker = [&](int id) {
        while (true) {
            size_t i = index.fetch_add(1);
            if (i >= jobs.size()) break;

            auto network = jobs[i];

            std::string bssid = network["bssid"].get<std::string>();
            int channel = network["channel"].get<int>();

            std::cout << "[worker " << id << "] processing "
                      << bssid << " ch " << channel << std::endl;

            pid_t pid = fork();

            if (pid == 0) {
                // Example: replace this with your real tool chain
                std::string cmd =
                    "echo setup " + bssid +
                    " && sleep 2 && echo running " + bssid;

                execlp("sh", "sh", "-c", cmd.c_str(), nullptr);
                _exit(1);
            }

            waitpid(pid, nullptr, 0);
        }
    };

    // ----------------------------
    // Spawn workers
    // ----------------------------
    std::vector<pid_t> workers;

    for (int i = 0; i < WORKERS; i++) {
        pid_t pid = fork();

        if (pid == 0) {
            worker(i);
            _exit(0);
        }

        workers.push_back(pid);
    }

    // ----------------------------
    // Wait for all workers
    // ----------------------------
    for (pid_t pid : workers) {
        waitpid(pid, nullptr, 0);
    }

    std::cout << "All tasks completed.\n";
} 

