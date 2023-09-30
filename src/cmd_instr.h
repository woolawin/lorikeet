#ifndef LK_CMD_INSTR
#define LK_CMD_INSTR

#include "core_types.h"

struct CommandInstr {
    InstructionID id;
    std::string name;
    std::string path;

    bool operator==(const CommandInstr& other) const;
    friend std::ostream& operator<<(std::ostream& os, const CommandInstr& instr);
};


#endif