//
// Created by 叶荣杰 on 2024/9/8.
//

#ifndef RVCAPP_REMOTE_CONTROL_H
#define RVCAPP_REMOTE_CONTROL_H

#endif //RVCAPP_REMOTE_CONTROL_H

#include <string>

#include "tbox_mqtt_handler.h"

/**
 * 远程控制基类
 */
class RemoteControl : public TboxMqttHandler {
public:
    /**
     * 控制指令
     * @param payload 数据
     * @param payload_len 数据长度
     */
    virtual void ControlCmd(const void *payload, int payload_len) = 0;

    /**
     * 开始时执行
     */
    virtual void OnStart() = 0;

    /**
     * 结束时执行
     */
    virtual void OnFinish() = 0;

    /**
     * 错误时执行
     */
    virtual void OnError() = 0;

    virtual ~RemoteControl() = default;
};