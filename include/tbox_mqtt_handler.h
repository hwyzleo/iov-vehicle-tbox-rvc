//
// Created by 叶荣杰 on 2024/9/8.
//

#ifndef RVCAPP_TBOX_MQTT_HANDLER_H
#define RVCAPP_TBOX_MQTT_HANDLER_H
class TboxMqttHandler {
public:
    /**
     * 处理消息
     * @param payload 数据
     * @param payload_len 数据长度
     */
    virtual void Handle(void *payload, int payload_len) = 0;

    virtual ~TboxMqttHandler() = default;
};
#endif //RVCAPP_TBOX_MQTT_HANDLER_H

