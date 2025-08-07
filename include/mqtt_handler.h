//
// Created by hwyz_leo on 2024/9/8.
//

#ifndef RVCAPP_MQTT_HANDLER_H
#define RVCAPP_MQTT_HANDLER_H
class MqttHandler {
public:
    /**
     * 处理消息
     * @param payload 数据
     * @param payload_len 数据长度
     */
    virtual void handle(const void *payload, int payload_len) = 0;

    virtual ~MqttHandler() = default;
};
#endif //RVCAPP_MQTT_HANDLER_H

