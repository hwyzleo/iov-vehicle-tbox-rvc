//
// Created by 叶荣杰 on 2024/9/8.
//
#include <iostream>

#include "find_vehicle.h"

/**
 * 寻车
 */
void FindVehicle::ControlCmd(const void *payload, int payload_len) {

}

void FindVehicle::OnStart() {

}

void FindVehicle::OnFinish() {

}

void FindVehicle::OnError() {

}

void FindVehicle::Handle(void *payload, int payload_len) {
    std::cout << "处理寻车指令: " << std::string(static_cast<char *>(payload), payload_len) << std::endl;
}