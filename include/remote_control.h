//
// Created by hwyz_leo on 2024/9/8.
//

#ifndef RVCAPP_REMOTE_CONTROL_H
#define RVCAPP_REMOTE_CONTROL_H
#include <string>

#include "mqtt_handler.h"

/**
 * 远程控制基类
 */
class RemoteControl : public MqttHandler {
public:
    /**
     * 控制指令
     * @param payload 数据
     * @param payload_len 数据长度
     */
    virtual void control_cmd(const void *payload, int payload_len) = 0;

    /**
     * 开始时执行
     */
    virtual void on_start() = 0;

    /**
     * 结束时执行
     */
    virtual void on_finish() = 0;

    /**
     * 错误时执行
     */
    virtual void on_error() = 0;

    virtual ~RemoteControl() = default;
};

#endif //RVCAPP_REMOTE_CONTROL_H