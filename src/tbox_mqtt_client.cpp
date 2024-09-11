//
// Created by 叶荣杰 on 2024/9/8.
//
#include <iostream>

#include "../third_party/include/spdlog/spdlog.h"

#include "tbox_mqtt_client.h"
#include "tbox_mqtt_config.h"
#include "find_vehicle.h"

TboxMqttClient::TboxMqttClient() : mosqpp::mosquittopp() {}

TboxMqttClient::~TboxMqttClient() {
    mosqpp::lib_cleanup();
}

TboxMqttClient &TboxMqttClient::GetInstance() {
    static TboxMqttClient instance;
    return instance;
}

bool TboxMqttClient::Start() {
    if (!is_started_) {
        spdlog::info("启动TBOX MQTT客户端");
        this->ConnectManage();
        is_started_ = true;
    }
    return is_started_;
}

void TboxMqttClient::Stop() {
    if (!is_started_) {
        return;
    }
    this->disconnect();
    mosqpp::lib_cleanup();
    is_started_ = false;
}

bool TboxMqttClient::IsConnected() const {
    return is_connected_;
}

bool TboxMqttClient::Publish(int &mid, const std::string &topic, const void *payload, int payload_len, int qos) {
    if (nullptr == payload) {
        return false;
    }
    if (!is_connected_) {
        return false;
    }
    spdlog::debug("转发TSP消息[{}]至主题[{}]", std::string(static_cast<const char *>(payload), payload_len), topic);
    int rc = mosquittopp::publish(&mid, topic.c_str(), payload_len, payload, qos, false);
    if (rc == MOSQ_ERR_SUCCESS) {
        cv_loop_.notify_all();
        return true;
    }
    return false;
}

bool TboxMqttClient::Subscribe(int &mid, const std::string &topic, std::unique_ptr<TboxMqttHandler> &&handler,
                               int qos) {
    if (!is_connected_) {
        return false;
    }
    if (topic.empty()) {
        return false;
    }
    int rc = this->subscribe(&mid, topic.c_str(), qos);
    spdlog::info("订阅主题[{}][{}]", mid, topic);
    if (rc != MOSQ_ERR_SUCCESS) {
        spdlog::warn("订阅主题[{}]失败[{}]", topic, rc);
        return false;
    }
    topic_handler_[topic] = std::move(handler);
    cv_loop_.notify_all();
    return true;
}

void TboxMqttClient::on_connect(int rc) {
    is_connecting_ = false;
    is_connected_ = (rc == MOSQ_ERR_SUCCESS);
    if (is_connected_) {
        spdlog::info("TBOX MQTT客户端连接成功");
        int mid = 0;
        std::unique_ptr<TboxMqttHandler> find_vehicle = std::make_unique<FindVehicle>();
        Subscribe(mid, "FIND_VEHICLE", std::move(find_vehicle), 1);
        is_subscribed_ = true;
    }
}

void TboxMqttClient::on_disconnect(int rc) {
    is_connecting_ = false;
    is_connected_ = false;
}

void TboxMqttClient::on_publish(int rc) {

}

void TboxMqttClient::on_message(const struct mosquitto_message *message) {
    spdlog::debug("收到消息主题[{}]内容[{}]", message->topic,
                  std::string(static_cast<char *>(message->payload), message->payloadlen));
    topic_handler_[message->topic]->Handle(message->payload, message->payloadlen);
}

void TboxMqttClient::on_subscribe(int mid, int qos_count, const int *granted_qos) {
    spdlog::info("订阅主题[{}]成功", mid);
}

void TboxMqttClient::on_unsubscribe(int mid) {
    spdlog::info("取消订阅主题[{}]成功", mid);
}

void TboxMqttClient::on_log(int level, const char *str) {

}

void TboxMqttClient::on_error() {

}

bool TboxMqttClient::Init() {
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

void TboxMqttClient::ConnectManage() {
    std::thread th([&]() {
        bool is_first_connect = true;
        while (is_started_) {
            if (!Init()) {
                spdlog::warn("TBOX MQTT客户端初始化失败");
                std::this_thread::sleep_for(std::chrono::seconds(kMqttReconnectIntervalSecond));
                continue;
            }
            if (!is_connected_ && !is_connecting_) {
                if (is_first_connect) {
                    is_first_connect = false;
                } else {
                    std::this_thread::sleep_for(std::chrono::seconds(kMqttReconnectIntervalSecond));
                }
                if (Connect()) {
                    is_connecting_ = true;
                }
            } else {
                this->loop();
            }
            std::unique_lock<std::mutex> lock(mtx_loop_);
            cv_loop_.wait_for(lock, std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::milliseconds(kMqttLoopIntervalMilliSecond)));
        }
    });
    connector.swap(th);
}

bool TboxMqttClient::Connect() {
    std::string sn;
    std::string vin;
    if (!GetDeviceInfo(sn, vin)) {
        return false;
    }
    if (!TboxMqttConfig::GetInstance().SetInfo(vin, sn)) {
        return false;
    }
    MqttConfig config = TboxMqttConfig::GetInstance().get_mqtt_config();
    spdlog::info("重置客户端ID");
    int rc = this->reinitialise(config.client_id.c_str(), true);
    if (rc != MOSQ_ERR_SUCCESS) {
        return false;
    }
    spdlog::info("设置用户名密码");
    rc = this->username_pw_set(config.username.c_str(), config.password.c_str());
    if (rc != MOSQ_ERR_SUCCESS) {
        return false;
    }
    spdlog::info("连接TBOX MQTT[{}:{}]", config.server_host, config.server_port);
    rc = mosquittopp::connect(config.server_host.c_str(), config.server_port, config.keepalive);
    if (rc != MOSQ_ERR_SUCCESS) {
        spdlog::warn("连接TBOX MQTT失败[{}]", rc);
        return false;
    }
    return true;
}

bool TboxMqttClient::GetDeviceInfo(std::string &sn, std::string &vin) const {
    sn.clear();
    vin.clear();
    // 当前先写死
    sn = "RVCAPP";
    vin = "RVCAPP";
    spdlog::info("获取SN及VIN");
    return true;
}