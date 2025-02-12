#pragma once

#include <vector>
#include "esphome/components/fan/fan.h"
#include "esphome/components/uart/uart.h"
#include "esphome/core/component.h"

namespace esphome {
namespace hubspace_fan {

class HubspaceFan : public fan::Fan, public Component, public uart::UARTDevice {
   public:
    void dump_config() override;
    void setup() override;
    void loop() override;
    fan::FanTraits get_traits() override;

   protected:
    long _last_send = 0;
    int _send_interval = 200;
    std::vector<uint8_t> _read_buffer;

    void control(const fan::FanCall &call) override;
    char calculate_checksum(std::vector<uint8_t>);
    void send(std::vector<uint8_t>);
};

}  // namespace hubspace_fan
}  // namespace esphome
