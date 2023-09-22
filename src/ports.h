#ifndef LK_PORTS
#define LK_PORTS

#include <vector>
#include <string>

struct File {
    std::string path;
    std::string name;
    bool can_execute;
};

class Disk {
    public:
    virtual std::vector<File> ls(const std::string& path) = 0;
};

class FileSystemDisk: public Disk {
    public:
    std::vector<File> ls(const std::string& path);
};

class Env {
    public:
    virtual std::string var(const std::string& name) = 0;
};

class ShellEnv: public Env {
    public:
    std::string var(const std::string& name);
};

#endif