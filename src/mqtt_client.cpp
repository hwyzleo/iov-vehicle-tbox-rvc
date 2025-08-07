//
// Created by hwyz_leo on 2024/9/8.
//
#include <iostream>
#include <string>

#include "spdlog/spdlog.h"
#include "utils.h"

#include "mqtt_client.h"
#include "find_vehicle.h"

MqttClient::MqttClient() : mosqpp::mosquittopp() {}

MqttClient::~MqttClient() {
    mosqpp::lib_cleanup();
}

MqttClient &MqttClient::get_instance() {
    static MqttClient instance;
    return instance;
}

bool MqttClient::load_config(const YAML::Node &config) {
    spdlog::info("加载MQTT客户端配置信息");
    if (config["mqtt"]) {
        if (config["mqtt"]["server"]) {
            if (config["mqtt"]["server"]["host"]) {
                server_host_ = config["mqtt"]["server"]["host"].as<std::string>();
            }
            if (config["mqtt"]["server"]["port"]) {
                server_port_ = config["mqtt"]["server"]["port"].as<std::uint16_t>();
            }
        }
        if (config["mqtt"]["keepalive"]) {
            keepalive_ = config["mqtt"]["keepalive"].as<std::uint16_t>();
        }
        if (config["mqtt"]["use-ssl"]) {
            use_ssl_ = config["mqtt"]["use-ssl"].as<bool>();
        }
        if (config["mqtt"]["reconnect-interval-second"]) {
            reconnect_interval_second_ = config["mqtt"]["reconnect-interval-second"].as<int>();
        }
        if (config["mqtt"]["username"]) {
            username_ = config["mqtt"]["username"].as<std::string>();
        }
        if (config["mqtt"]["password"]) {
            username_ = config["mqtt"]["password"].as<std::string>();
        }
    }
    return true;
}

bool MqttClient::start() {
    if (!is_started_) {
        spdlog::info("启动TBOX MQTT客户端");
        this->connect_manage();
        is_started_ = true;
    }
    return is_started_;
}

void MqttClient::stop() {
    if (!is_started_) {
        return;
    }
    this->disconnect();
    mosqpp::lib_cleanup();
    is_started_ = false;
}

bool MqttClient::is_connected() const {
    return is_connected_;
}

bool MqttClient::publish(int &mid, const std::string &topic, const void *payload, int payload_len, int qos) {
    if (nullptr == payload) {
        return false;
    }
    if (!is_connected_) {
        return false;
    }
    std::string base64_payload = hwyz::Utils::base64_encode(
            std::string(static_cast<const char *>(payload), payload_len));
    int rc = mosquittopp::publish(&mid, topic.c_str(), static_cast<int>(base64_payload.length()),
                                  base64_payload.c_str(), qos, false);
    spdlog::debug("发送[{}]远控APP消息[{}]至主题[{}]", mid,
                  std::string(static_cast<const char *>(payload), payload_len), topic);
    if (rc == MOSQ_ERR_SUCCESS) {
        cv_loop_.notify_all();
        return true;
    }
    return false;
}

void MqttClient::on_connect(int rc) {
    is_connecting_ = false;
    is_connected_ = (rc == MOSQ_ERR_SUCCESS);
    if (is_connected_) {
        spdlog::info("TBOX MQTT客户端连接成功");
        int mid = 0;
        subscribe_topic(mid, "APP/FIND_VEHICLE", FindVehicle::get_instance(), 1);
        // 特殊订阅，先满足测试场景
        subscribe_topic(mid, "TSP/FIND_VEHICLE", FindVehicle::get_instance(), 1);
        is_subscribed_ = true;
    }
}

void MqttClient::on_disconnect(int rc) {
    is_connecting_ = false;
    is_connected_ = false;
}

void MqttClient::on_publish(int rc) {
    spdlog::debug("发送[{}]远控APP消息成功", rc);
}

void MqttClient::on_message(const struct mosquitto_message *message) {
    spdlog::info("收到消息主题[{}]内容[{}]", message->topic,
                 std::string(static_cast<char *>(message->payload), message->payloadlen));
    std::string payload = hwyz::Utils::base64_decode(
            std::string(static_cast<char *>(message->payload), message->payloadlen));
    topic_handler_[message->topic]->handle(payload.c_str(), static_cast<int>(payload.length()));
}

void MqttClient::on_subscribe(int mid, int qos_count, const int *granted_qos) {
    spdlog::info("订阅[{}]主题成功", mid);
}

void MqttClient::on_unsubscribe(int mid) {
    spdlog::info("取消订阅主题[{}]成功", mid);
}

void MqttClient::on_log(int level, const char *str) {

}

void MqttClient::on_error() {

}

bool MqttClient::init() {
    if (!is_inited_) {
        spdlog::info("初始化TBOX MQTT客户端");
        int rc = mosqpp::lib_init();
        if (rc == MOSQ_ERR_SUCCESS) {
            spdlog::info("TBOX MQTT客户端初始化成功");
            is_inited_ = true;
        }
    }
    return is_inited_;
}

void MqttClient::connect_manage() {
    std::thread th([&]() {
        bool is_first_connect = true;
        while (is_started_) {
            if (!init()) {
                spdlog::warn("TBOX MQTT客户端初始化失败");
                std::this_thread::sleep_for(std::chrono::seconds(reconnect_interval_second_));
                continue;
            }
            if (!is_connected_ && !is_connecting_) {
                if (is_first_connect) {
                    is_first_connect = false;
                } else {
                    std::this_thread::sleep_for(std::chrono::seconds(reconnect_interval_second_));
                }
                if (connect()) {
                    is_connecting_ = true;
                }
            } else {
                this->loop();
            }
            std::unique_lock<std::mutex> lock(mtx_loop_);
            cv_loop_.wait_for(lock, std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::milliseconds(loop_interval_milli_second_)));
        }
    });
    connector.swap(th);
}

bool MqttClient::connect() {
    spdlog::info("重置客户端ID");
    int rc = this->reinitialise(client_id_.c_str(), true);
    if (rc != MOSQ_ERR_SUCCESS) {
        return false;
    }
    spdlog::info("设置用户名密码");
    rc = this->username_pw_set(username_.c_str(), password_.c_str());
    if (rc != MOSQ_ERR_SUCCESS) {
        return false;
    }
    spdlog::info("连接TBOX MQTT[{}:{}]", server_host_, server_port_);
    rc = mosquittopp::connect(server_host_.c_str(), server_port_, keepalive_);
    if (rc != MOSQ_ERR_SUCCESS) {
        spdlog::warn("连接TBOX MQTT失败[{}]", rc);
        return false;
    }
    return true;
}

bool MqttClient::subscribe_topic(int &mid, const std::string &topic, MqttHandler &handler, int qos) {
    if (!is_connected_) {
        return false;
    }
    if (topic.empty()) {
        return false;
    }
    int rc = this->subscribe(&mid, topic.c_str(), qos);
    spdlog::info("订阅[{}]主题[{}]QOS[{}]", mid, topic, qos);
    if (rc != MOSQ_ERR_SUCCESS) {
        spdlog::warn("订阅主题[{}]失败[{}]", topic, rc);
        return false;
    }
    topic_handler_[topic] = &handler;
    cv_loop_.notify_all();
    return true;
}