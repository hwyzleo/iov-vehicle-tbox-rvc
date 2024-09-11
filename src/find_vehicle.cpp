//
// Created by 叶荣杰 on 2024/9/8.
//
#include <iostream>

#include "../third_party/include/spdlog/spdlog.h"

#include "find_vehicle.h"
#include <cstdlib>

/**
 * 寻车
 */
void FindVehicle::ControlCmd(const void *payload, int payload_len) {
    system("python3 /home/jetson/hwyz/test.py");
}

void FindVehicle::OnStart() {

}

void FindVehicle::OnFinish() {

}

void FindVehicle::OnError() {

}

void FindVehicle::Handle(void *payload, int payload_len) {
    spdlog::debug("处理寻车指令[{}]", std::string(static_cast<char *>(payload), payload_len));
    ControlCmd(payload, payload_len);
}