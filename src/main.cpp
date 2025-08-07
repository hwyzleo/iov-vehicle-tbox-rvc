//
// Created by hwyz_leo on 2024/9/8.
//
#include "application.h"
#include "spdlog/spdlog.h"

#include "mqtt_client.h"

class MainApplication : public hwyz::Application {
protected:
    bool initialize() override {
        if (!MqttClient::get_instance().load_config(getConfig())) {
            return false;
        }
        return true;
    }

    void cleanup() override {
        MqttClient::get_instance().stop();
    }

    int execute() override {
        system("python3 /home/jetson/hwyz/rvc.py");
        // 启动MQTT客户端
        MqttClient::get_instance().start();

        spdlog::info("主函数运行");
        return 0;
    }
};

APPLICATION_ENTRY(MainApplication)