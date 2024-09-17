//
// Created by 叶荣杰 on 2024/9/8.
//
#include <iostream>
#include <thread>

#include "../third_party/include/spdlog/spdlog.h"
#include "../third_party/include/spdlog/sinks/stdout_color_sinks.h"
#include "../third_party/include/spdlog/sinks/basic_file_sink.h"

#include "main.h"
#include "tbox_mqtt_client.h"
#include "find_vehicle.h"

// 主函数
int main() {
    std::string log_path = "/home/jetson/hwyz/iov-vehicle-tbox-rvc/build/log.txt";
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_path, true);
    file_sink->set_level(spdlog::level::debug);
    auto logger = std::make_shared<spdlog::logger>("file_logger", file_sink);
    logger->set_level(spdlog::level::debug);
    spdlog::set_default_logger(logger);
    spdlog::flush_every(std::chrono::seconds(5));
//    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
//    console_sink->set_level(spdlog::level::debug);
//    auto logger = std::make_shared<spdlog::logger>("console", console_sink);
//    spdlog::set_default_logger(logger);
    spdlog::info("主函数运行");
    system("python3 /home/jetson/hwyz/rvc.py");
    // 启动TBOX MQTT客户端
    TboxMqttClient::GetInstance().Start();
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}