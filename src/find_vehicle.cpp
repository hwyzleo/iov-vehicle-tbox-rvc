//
// Created by hwyz_leo on 2024/9/8.
//
#include <iostream>
#include <cstdlib>
#include <map>
#include <future>

#include "spdlog/spdlog.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

#include "find_vehicle.h"
#include "tbox_mqtt_client.h"

FindVehicle::FindVehicle() {
}

FindVehicle::~FindVehicle() {

}

FindVehicle &FindVehicle::get_instance() {
    static FindVehicle instance;
    return instance;
}

void FindVehicle::control_cmd(const void *payload, int payload_len) {
    const char *char_payload = static_cast<const char *>(payload);
    spdlog::info("收到寻车指令[{}]", std::string(char_payload, payload_len));
    std::string json_string(char_payload, payload_len);
    json json_object;
    try {
        json_object = json::parse(json_string);
    } catch (json::parse_error &e) {
        std::cerr << "JSON解析错误: " << e.what() << std::endl;
        return;
    }
    spdlog::debug("解析寻车指令[{}]", cmd_id_);
    cmd_id_ = json_object["cmdId"];
    if (json_object.contains("state")) {
        if (json_object["state"] == 2) {
            std::this_thread::sleep_for(std::chrono::seconds(10));
            system("python3 /home/jetson/hwyz/find_vehicle_end.py");
            on_finish();
        }
    } else {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        system("python3 /home/jetson/hwyz/find_vehicle.py");
        on_start();
    }
}

void FindVehicle::on_start() {
    int mid = 0;
    std::map<std::string, json> params;
    params["type"] = "FIND_VEHICLE";
    params["state"] = 2;
    params["cmdId"] = cmd_id_;
    params["ts"] = static_cast<long>(std::time(nullptr));
    json j = params;
    std::string params_json = j.dump();
    TboxMqttClient::get_instance().publish(
            mid,
            "TSP/FIND_VEHICLE",
            params_json.c_str(),
            static_cast<int>(params_json.length()),
            1
    );
    spdlog::debug("寻车指令[{}]开始执行", cmd_id_);
}

void FindVehicle::on_finish() {
    int mid = 0;
    std::map<std::string, json> params;
    params["type"] = "FIND_VEHICLE";
    params["state"] = 3;
    params["cmdId"] = cmd_id_;
    params["ts"] = static_cast<long>(std::time(nullptr));
    json j = params;
    std::string params_json = j.dump();
    TboxMqttClient::get_instance().publish(
            mid,
            "TSP/FIND_VEHICLE",
            params_json.c_str(),
            static_cast<int>(params_json.length()),
            1
    );
    spdlog::debug("寻车指令[{}]执行完成", cmd_id_);
}

void FindVehicle::on_error() {
    int mid = 0;
    std::map<std::string, json> params;
    params["type"] = "FIND_VEHICLE";
    params["state"] = 4;
    params["failure"] = 1;
    params["cmdId"] = cmd_id_;
    json j = params;
    std::string params_json = j.dump();
    TboxMqttClient::get_instance().publish(
            mid,
            "TSP/FIND_VEHICLE",
            params_json.c_str(),
            static_cast<int>(params_json.length()),
            1
    );
    spdlog::warn("寻车指令[{}]执行失败", cmd_id_);
}

void FindVehicle::handle(const void *payload, int payload_len) {
    spdlog::debug("处理寻车指令[{}]", std::string(static_cast<const char *>(payload), payload_len));
    FindVehicle::get_instance().control_cmd(payload, payload_len);
}