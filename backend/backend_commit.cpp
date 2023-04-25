#include "logger.h"
#include "processor.h"

/**
 * @brief 处理前端流出的指令
 *
 * @param inst 前端将要流出的指令（在流出buffer里面）
 * @return true 后端接受该指令
 * @return false 后端拒绝该指令
 */
bool Backend::dispatchInstruction([[maybe_unused]] const Instruction &inst) {
    if (!rob.canPush()) return false;

    // TODO: Check rob and reservation station is available for push.
    // NOTE: use getFUType to get instruction's target FU
    // NOTE: FUType::NONE only goes into ROB but not Reservation Stations

    switch (getFUType(inst)) {
        case FUType::NONE:
            rob.push(inst, true);
            return true;
        case FUType::ALU:
            if (rsALU.hasEmptySlot()) {
                unsigned robIdx = rob.push(inst, false);
                rsALU.insertInstruction(inst, robIdx, regFile, rob);
                regFile->markBusy(inst.getRd(), robIdx);
                return true;
            }
            return false;
        case FUType::BRU:
            if (rsBRU.hasEmptySlot()) {
                unsigned robIdx = rob.push(inst, false);
                rsBRU.insertInstruction(inst, robIdx, regFile, rob);
                regFile->markBusy(inst.getRd(), robIdx);
                return true;
            }
            return false;
        case FUType::MUL:
            if (rsMUL.hasEmptySlot()) {
                unsigned robIdx = rob.push(inst, false);
                rsMUL.insertInstruction(inst, robIdx, regFile, rob);
                regFile->markBusy(inst.getRd(), robIdx);
                return true;
            }
            return false;
        case FUType::DIV:
            if (rsDIV.hasEmptySlot()) {
                unsigned robIdx = rob.push(inst, false);
                rsDIV.insertInstruction(inst, robIdx, regFile, rob);
                regFile->markBusy(inst.getRd(), robIdx);
                return true;
            }
            return false;
        case FUType::LSU:
            if (rsLSU.hasEmptySlot()) {
                unsigned robIdx = rob.push(inst, false);
                rsLSU.insertInstruction(inst, robIdx, regFile, rob);
                regFile->markBusy(inst.getRd(), robIdx);
                return true;
            }
            return false;
        default:
            Logger::Error("Instruction dispatch not implemented!");
            std::__throw_runtime_error("Instruction dispatch not implemented!");
            break;
    }
    
    return false;
}

/**
 * @brief 后端完成指令提交
 *
 * @param entry 被提交的 ROBEntry
 * @param frontend 前端，用于调用前端接口
 * @return true 提交了 EXTRA::EXIT
 * @return false 其他情况
 */
bool Backend::commitInstruction([[maybe_unused]] const ROBEntry &entry,
                                [[maybe_unused]] Frontend &frontend) {
    // TODO: Commit instructions here. Set executeExit when committing
    // EXTRA::EXIT NOTE: Be careful about Store Buffer! NOTE: Be careful about
    // flush!
    if (entry.inst == EXTRA::EXIT)
        return true;

    // TODO: frontend.bpuBackendUpdate();
    unsigned robIdx = rob.getPopPtr();

    std::stringstream ss;
    ss << entry.inst;
    Logger::Info("************ After commit, inst is %s", ss.str().c_str());

    unsigned rd = entry.inst.getRd();
        
    if (entry.inst == RV32I::SB || entry.inst == RV32I::SH || entry.inst == RV32I::SW) {
        auto addr = entry.state.result;
        auto buf = storeBuffer.query(addr);
        if (!buf.has_value()) {
            Logger::Error("Store address mismatch!");
            std::__throw_runtime_error("Store address mismatch!");
        }
        Logger::Info("LSU addr: %08x, res: %08x", entry.state.result, buf.value());
        data[(addr - 0x80400000) >> 2] = buf.value();
    } else {
        regFile->write(rd, entry.state.result, robIdx);
        Logger::Info("************ After commit, reg[%s] = %08x", xreg_name[rd].c_str(), entry.state.result);
    }

    rob.pop();
    if (entry.state.actualTaken && entry.state.mispredict) {
        Logger::Info("-----------------⚠Mispredict!⚠----------------");
        frontend.jump(entry.state.jumpTarget);
        flush();
        return false;
    }

    return false;
}