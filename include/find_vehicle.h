//
// Created by 叶荣杰 on 2024/9/8.
//

#ifndef RVCAPP_FIND_VEHICLE_H
#define RVCAPP_FIND_VEHICLE_H

#endif //RVCAPP_FIND_VEHICLE_H

#include "remote_control.h"

class FindVehicle : public RemoteControl {
public:
    /**
     * 控制指令
     * @param payload 数据
     * @param payload_len 数据长度
     */
    void ControlCmd(const void *payload, int payload_len) override;

    /**
     * 开始时执行
     */
    void OnStart() override;

    /**
     * 结束时执行
     */
    void OnFinish() override;

    /**
     * 错误时执行
     */
    void OnError() override;

    /**
     * 处理消息
     * @param payload 数据
     * @param payload_len 数据长度
     */
    void Handle(void *payload, int payload_len) override;

    virtual ~FindVehicle() = default;
};