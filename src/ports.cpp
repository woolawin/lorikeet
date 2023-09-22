
#include <string>
#include <vector>
#include <filesystem>
#include "ports.h"

namespace fs = std::filesystem;

std::vector<File> FileSystemDisk::ls(const std::string& path) {
    std::vector<File> files;
    for (const fs::directory_entry& entry : fs::directory_iterator(path)) {
        if (!entry.is_regular_file()) {
            continue;
        }
        const fs::perms perms = fs::status(entry.path()).permissions();
        const bool exec_perm = (perms & fs::perms::owner_exec) != fs::perms::owner_exec;
        files.push_back({ .path = "", .name = entry.path().filename(), .can_execute = exec_perm });
    }
    return files;
}


std::string ShellEnv::var(const std::string& name) {
    return std::getenv(name.c_str());
}