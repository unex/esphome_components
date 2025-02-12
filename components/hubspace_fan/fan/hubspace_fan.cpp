#include "hubspace_fan.h"
#include <string>
#include <vector>
#include "esphome/core/log.h"

namespace esphome {
namespace hubspace_fan {

static const char* TAG = "hubspace_fan.fan";

void logVector(const std::string& msg, std::vector<uint8_t>& data) {
    std::string hexStr;
    for (uint8_t c : data) {
        char buf[4];
        snprintf(buf, sizeof(buf), "%02X ", static_cast<unsigned char>(c));
        hexStr += buf;
    }
    ESP_LOGD(TAG, "%s %s", msg.c_str(), hexStr.c_str());
}

uint8_t speed_to_percentage(int speed) {
    if (speed == 0) return 0;
    return speed * 10 + 10;
}

int percentage_to_speed(uint8_t percentage) {
    if (percentage == 0) return 0;
    return (percentage - 10) / 10;
}

void HubspaceFan::setup() {}

void HubspaceFan::loop() {
    if (esphome::millis() - _last_send >= _send_interval) {
        std::vector<uint8_t> toSend = {0x20, 0x01, 0x00, 0x00};
        send(toSend);

        ESP_LOGD(TAG, "Sent Heartbeat");
    }

    while (available() > 0) {
        uint8_t received;
        read_byte(&received);

        if (_read_buffer.size() == 0 && received != 0x20)
            continue;

        else if (_read_buffer.size() == 11) {
            if (calculate_checksum(_read_buffer) == received) {
                logVector("Recieved:", _read_buffer);

                uint8_t speed = percentage_to_speed(_read_buffer[4]);

                if (speed != this->speed) {
                    this->state = speed > 0;
                    this->speed = speed;

                    this->publish_state();
                }

                uint8_t dir_raw = _read_buffer[10];
                fan::FanDirection direction;

                if (dir_raw == 0x00)
                    direction = fan::FanDirection::FORWARD;
                else if (dir_raw == 0x80)
                    direction = fan::FanDirection::REVERSE;
                if (direction != this->direction) {
                    this->direction = direction;

                    this->publish_state();
                }
            }

            _read_buffer.clear();

            continue;
        }

        _read_buffer.push_back(received);
    }
}

void HubspaceFan::send(std::vector<uint8_t> toSend) {
    toSend.push_back(calculate_checksum(toSend));

    write_array(toSend);

    _last_send = esphome::millis();

    logVector("Sent:", toSend);
}

char HubspaceFan::calculate_checksum(std::vector<uint8_t> data) {
    char checksum = 0;
    for (size_t i = 0; i < data.size(); i++) {
        checksum ^= data[i];
    }
    return checksum;
}

fan::FanTraits HubspaceFan::get_traits() {
    return fan::FanTraits(false, true, true, 9);
}

void HubspaceFan::control(const fan::FanCall& call) {
    std::vector<uint8_t> toSend;

    int speed = -1;

    if (call.get_state().has_value()) {
        if (!*call.get_state()) speed = 0;
    }

    if (call.get_speed().has_value()) {
        speed = *call.get_speed();
    }

    if (speed != -1 && speed != this->speed) {
        toSend = {0x20, 0x02, speed_to_percentage(speed), 0x00};
        send(toSend);
    }

    if (call.get_direction().has_value()) {
        fan::FanDirection direction = *call.get_direction();

        toSend = {0x20, 0x04, static_cast<uint8_t>(direction), 0x00};
        send(toSend);
    }
}

void HubspaceFan::dump_config() { LOG_FAN("", "HubspaceFan", this); }

}  // namespace hubspace_fan
}  // namespace esphome
