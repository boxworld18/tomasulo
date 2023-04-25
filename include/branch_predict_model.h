// Optional TODO: Change this file to whatever you like!
// NOTE: Please keep these signals that are already in used.
struct BpuUpdateData {
    unsigned pc;
    bool isCall, isReturn, isBranch, branchTaken;
    unsigned jumpTarget;
};

struct BranchPredictBundle {
    bool predictJump = false;
    unsigned predictTarget = 0;
};

struct BTBSlot {
    bool valid = false;
    short predict = 0;
    unsigned index = 0;
    unsigned target = 0;
};

constexpr int BTB_BIT = 12;
constexpr int BTB_SIZE = 1 << BTB_BIT;
constexpr int BTB_SLOT = BTB_SIZE - 1;