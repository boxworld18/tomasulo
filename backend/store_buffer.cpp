#include "store_buffer.h"

#include "logger.h"

StoreBuffer::StoreBuffer() {
    pushPtr = popPtr = 0;
    for (auto &slot : buffer) {
        slot.valid = false;
    }
}

/**
 * @brief 向ROB推入新项
 * 
 * @param addr store地址
 * @param value store值
 */
void StoreBuffer::push(unsigned addr, unsigned value) {
    buffer[pushPtr].storeAddress = addr;
    buffer[pushPtr].storeData = value;
    buffer[pushPtr].valid = true;
    Logger::Info("Store buffer push:");
    Logger::Info("Index: %u", pushPtr);
    Logger::Info("Address: %08x, value: %u\n", addr, value);
    pushPtr++;
    if (pushPtr == ROB_SIZE) {
        pushPtr -= ROB_SIZE;
    }
}

/**
 * @brief 从StoreBuffer中弹出一项，需保证Store Buffer中有项目
 * 
 * @return StoreBufferSlot 弹出的StoreBuffer项目
 */
StoreBufferSlot StoreBuffer::pop() {
    auto ret = buffer[popPtr];
    Logger::Info("Store buffer pop:");
    Logger::Info("Index: %u", popPtr);
    Logger::Info("Address: %08x, value: %u\n", ret.storeAddress, ret.storeData);
    buffer[popPtr].valid = false;
    popPtr++;
    if (popPtr == ROB_SIZE) {
        popPtr -= ROB_SIZE;
    }
    return ret;
}

/**
 * @brief 清空Store Buffer状态
 * 
 */
void StoreBuffer::flush() {
    pushPtr = popPtr = 0;
    for (auto &slot : buffer) {
        slot.valid = false;
    }
}

/**
 * @brief 查询StoreBuffer中的内容
 * 
 * @param addr 地址
 * @return std::optional<unsigned> 如果在Store Buffer中命中，返回对应值，否则返回std::nullopt
 */
std::optional<unsigned> StoreBuffer::query(unsigned addr) {
    unsigned p = pushPtr - 1;
    if (pushPtr == 0) {
        p += ROB_SIZE;
    }
    // while (p != popPtr) { 
    do {
        if (buffer[p].valid &&
            (buffer[p].storeAddress & 0xFFFFFFFCu) == (addr & 0xFFFFFFFCu)) {
            return std::make_optional(buffer[p].storeData);
        }
        if (p == 0) {
            p += ROB_SIZE;
        }
        p--;
    // } 
    } while (p != popPtr);
    if (buffer[p].valid &&
        (buffer[p].storeAddress & 0xFFFFFFFCu) == (addr & 0xFFFFFFFCu)) {
        return std::make_optional(buffer[p].storeData);
    }
    return std::nullopt;
}
