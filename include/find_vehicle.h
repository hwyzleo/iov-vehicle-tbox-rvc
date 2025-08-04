//
// Created by hwyz_leo on 2024/9/8.
//

#ifndef RVCAPP_FIND_VEHICLE_H
#define RVCAPP_FIND_VEHICLE_H

#endif //RVCAPP_FIND_VEHICLE_H

#include "remote_control.h"

class FindVehicle : public RemoteControl {
public:
    /**
     * 析构虚函数
     */
    ~FindVehicle() override;

    /**
     * 获取单例
     * @return 单例
     */
    static FindVehicle &get_instance();

    /**
     * 防止对象被复制
     */
    FindVehicle(const FindVehicle &) = delete;

    /**
     * 防止对象被赋值
     * @return
     */
    FindVehicle &operator=(const FindVehicle &) = delete;

    /**
     * 控制指令
     * @param payload 数据
     * @param payload_len 数据长度
     */
    void control_cmd(const void *payload, int payload_len) override;

    /**
     * 开始时执行
     */
    void on_start() override;

    /**
     * 结束时执行
     */
    void on_finish() override;

    /**
     * 错误时执行
     */
    void on_error() override;

    /**
     * 处理消息
     * @param payload 数据
     * @param payload_len 数据长度
     */
    void handle(const void *payload, int payload_len) override;
    
private:
    FindVehicle();

private:

    // 指令ID
    std::string cmd_id_;
};