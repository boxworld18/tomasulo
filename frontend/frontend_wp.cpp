#include "with_predict.h"

FrontendWithPredict::FrontendWithPredict(const std::vector<unsigned> &inst)
    : Frontend(inst) {
    // Optional TODO: initialize your prediction structures here.
    for (int i = 0; i < BTB_SIZE; i++)
        btb[i] = {false, 0, 0, 0};
}

unsigned getBTBSlot(unsigned pc) { return (pc >> 2) & BTB_SLOT; };
unsigned getBTBIndex(unsigned pc) { return (pc >> (BTB_BIT + 2)); }

/**
 * @brief 获取指令的分支预测结果，分支预测时需要
 * 
 * @param pc 指令的pc
 * @return BranchPredictBundle 分支预测的结构
 */
BranchPredictBundle FrontendWithPredict::bpuFrontendUpdate(unsigned int pc) {
    // Optional TODO: branch predictions
    BranchPredictBundle result;
    result.predictJump = false;
    result.predictTarget = pc + 4;

    BTBSlot slot = btb[getBTBSlot(pc)];
    if (!slot.valid || slot.index != getBTBIndex(pc)) return result;
    
    if (slot.predict & 2) {
        result.predictJump = true;
        result.predictTarget = slot.target;
        return result;
    }

    return result;
}

/**
 * @brief 用于计算NextPC，分支预测时需要
 * 
 * @param pc 
 * @return unsigned 
 */
unsigned FrontendWithPredict::calculateNextPC(unsigned pc) const {
    // Optional TODO: branch predictions
    BTBSlot slot = btb[getBTBSlot(pc)];
    if (!slot.valid || slot.index != getBTBIndex(pc)) return pc + 4;
    
    if (slot.predict & 2)
        return slot.target;

    return pc + 4;
}

/**
 * @brief 用于后端提交指令时更新分支预测器状态，分支预测时需要
 * 
 * @param x 
 */
void FrontendWithPredict::bpuBackendUpdate(const BpuUpdateData &x) {
    // Optional TODO: branch predictions
    if (!x.isBranch) return;

    unsigned slot_id = getBTBSlot(x.pc);
    unsigned btb_idx = getBTBIndex(x.pc);
    BTBSlot &slot = btb[slot_id];
    if (!slot.valid || slot.index != btb_idx) {
        slot.index = btb_idx;
        slot.predict = x.branchTaken ? 3 : 0;
    } else {
        if (x.branchTaken)
            slot.predict = (slot.predict == 0) ? 1 : 3;
        else
            slot.predict = (slot.predict == 3) ? 2 : 0;
    }
    slot.valid = true;
    slot.target = x.jumpTarget;

}
