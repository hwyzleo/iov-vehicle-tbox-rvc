//
// Created by hwyz_leo on 2024/9/8.
//
#include <iostream>
#include <thread>

#include "../third_party/include/spdlog/spdlog.h"
#include "../third_party/include/spdlog/sinks/stdout_color_sinks.h"
#include "../third_party/include/spdlog/sinks/basic_file_sink.h"
#include "../third_party/include/yaml-cpp/yaml.h"

#include "common_function.h"
#include "main.h"
#include "tbox_mqtt_client.h"
#include "find_vehicle.h"

// 主函数
int main() {
    // 加载配置
    std::string file_path = CommonFunction::get_config_file_path();
    YAML::Node config = YAML::LoadFile(file_path);
    // 初始化日志
    init_logger(config);
    spdlog::info("主函数运行");
    system("python3 /home/jetson/hwyz/rvc.py");
    // 启动TBOX MQTT客户端
    TboxMqttClient::get_instance().start();
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void init_logger(const YAML::Node &config) {
    std::string logger_type = config["logger"]["type"].as<std::string>();
    if (logger_type == "file") {
        std::string logger_path = config["logger"]["path"].as<std::string>();
        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logger_path, true);
        file_sink->set_level(spdlog::level::debug);
        auto logger = std::make_shared<spdlog::logger>("file_logger", file_sink);
        logger->set_level(spdlog::level::debug);
        spdlog::set_default_logger(logger);
        spdlog::flush_every(std::chrono::seconds(5));
    } else {
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(spdlog::level::debug);
        auto logger = std::make_shared<spdlog::logger>("console", console_sink);
        logger->set_level(spdlog::level::debug);
        spdlog::set_default_logger(logger);
    }
}