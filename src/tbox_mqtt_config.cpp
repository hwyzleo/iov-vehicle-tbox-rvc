//
// Created by 叶荣杰 on 2024/9/8.
//
#include <iostream>

#include "../third_party/include/spdlog/spdlog.h"

#include "tbox_mqtt_config.h"

TboxMqttConfig::TboxMqttConfig() : mqtt_config_() {
    mqtt_config_.subscribe_topics = subscribe_topics_;
}

TboxMqttConfig::~TboxMqttConfig() = default;

TboxMqttConfig &TboxMqttConfig::GetInstance() {
    static TboxMqttConfig instance;
    return instance;
}

bool TboxMqttConfig::SetInfo(const std::string &server_host, const std::string &username, const std::string &client_id) {
    spdlog::info("设置TBOX MQTT配置信息");
    if (server_host.empty() || username.empty() || client_id.empty()) {
        return false;
    }
    mqtt_config_.server_host = server_host;
    mqtt_config_.server_port = 1884;
    mqtt_config_.username = username;
    mqtt_config_.client_id = client_id;
    return GeneratePassword();
}

MqttConfig TboxMqttConfig::get_mqtt_config() const {
    return mqtt_config_;
}

bool TboxMqttConfig::GeneratePassword() {
    // 临时默认密码
    mqtt_config_.password = "111111";
    return true;
}