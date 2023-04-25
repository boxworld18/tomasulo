#pragma once
#include <memory>
#include <sstream>

#include "instructions.h"
#include "issue_slot.h"
#include "logger.h"
#include "register_file.h"
#include "rob.h"

template <unsigned size>
class ReservationStation {
    IssueSlot buffer[size];

public:
    ReservationStation();
    bool hasEmptySlot() const;
    void insertInstruction(const Instruction &inst,
                           unsigned robIdx,
                           std::shared_ptr<RegisterFile> regFile,
                           const ReorderBuffer &reorderBuffer);
    void wakeup(const ROBStatusWritePort &x);
    bool canIssue() const;
    IssueSlot issue();
    void flush();
};

template <unsigned size>
ReservationStation<size>::ReservationStation() {
    for (auto &slot : buffer) {
        slot.busy = false;
    }
}

template <unsigned size>
bool ReservationStation<size>::hasEmptySlot() const {
    for (auto const &slot : buffer) {
        if (!slot.busy) return true;
    }
    return false;
}

template <unsigned size>
void ReservationStation<size>::insertInstruction(
    const Instruction &inst,
    unsigned robIdx,
    std::shared_ptr<RegisterFile> regFile,
    const ReorderBuffer &reorderBuffer) {
    for (auto &slot : buffer) {
        if (slot.busy) continue;
        // TODO: Dispatch instruction to this slot
        unsigned rs1 = inst.getRs1();
        unsigned rs2 = inst.getRs2();
        unsigned rd = inst.getRd();

        if (!regFile->isBusy(rs1)) {
            slot.readPort1.value = regFile->read(rs1);
            slot.readPort1.waitForWakeup = false;
        } else {
            unsigned regRobIdx = regFile->getBusyIndex(rs1);
            slot.readPort1.robIdx = regRobIdx;
            if (reorderBuffer.checkReady(regRobIdx)) {
                slot.readPort1.value = reorderBuffer.read(regRobIdx);
                slot.readPort1.waitForWakeup = false;
            } else {
                slot.readPort1.waitForWakeup = true;
            }
        }

        if (!regFile->isBusy(rs2)) {
            slot.readPort2.value = regFile->read(rs2);
            slot.readPort2.waitForWakeup = false;
        } else {
            unsigned regRobIdx = regFile->getBusyIndex(rs2);
            slot.readPort2.robIdx = regRobIdx;
            if (reorderBuffer.checkReady(regRobIdx)) {
                slot.readPort2.value = reorderBuffer.read(regRobIdx);
                slot.readPort2.waitForWakeup = false;
            } else {
                slot.readPort2.waitForWakeup = true;
            }
        }   

        slot.inst = inst;
        slot.robIdx = robIdx;
        slot.busy = true;
        
        return;

        Logger::Error("Dispatching instruction is not implemented");
        std::__throw_runtime_error(
            "Dispatching instruction is not implemented");
    }
    Logger::Error("ReservationStation::insertInstruction no empty slots!");
    std::__throw_runtime_error(
        "No empty slots when inserting instruction into reservation "
        "station.");
}

template <unsigned size>
void ReservationStation<size>::wakeup(
    [[maybe_unused]] const ROBStatusWritePort &x) {
    // TODO: Wakeup instructions according to ROB Write
    for (auto &slot : buffer) {
        if (!slot.busy) continue;
        if (slot.readPort1.waitForWakeup && slot.readPort1.robIdx == x.robIdx) {
            slot.readPort1.value = x.result;
            slot.readPort1.waitForWakeup = false;
        }
        if (slot.readPort2.waitForWakeup && slot.readPort2.robIdx == x.robIdx) {
            slot.readPort2.value = x.result;
            slot.readPort2.waitForWakeup = false;
        }
    }
    return;
    Logger::Error("Wakeup not implemented");
    std::__throw_runtime_error("Wakeup not implemented");
}

template <unsigned size>
bool ReservationStation<size>::canIssue() const {
    // TODO: Decide whether an issueSlot is ready to issue.
    for (auto const &slot : buffer)
        if (slot.busy && !slot.readPort1.waitForWakeup && !slot.readPort2.waitForWakeup) return true;
    return false;
}

template <unsigned size>
IssueSlot ReservationStation<size>::issue() {
    // TODO: Issue a ready issue slot and remove it from reservation station.
    for (auto &slot : buffer) {
        if (slot.busy && !slot.readPort1.waitForWakeup && !slot.readPort2.waitForWakeup) {
            IssueSlot ret = slot;
            slot.busy = false;
            return ret;
        }
    }
    Logger::Error("No available slots for issuing");
    std::__throw_runtime_error("No available slots for issuing");
}

template <unsigned size>
void ReservationStation<size>::flush() {
    for (auto &slot : buffer) {
        slot.busy = false;
    }
}