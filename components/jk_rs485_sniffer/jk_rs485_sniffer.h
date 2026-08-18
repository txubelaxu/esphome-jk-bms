#pragma once

#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"
#include "esphome/components/uart/uart.h"
#include "talk_pin.h"



namespace esphome {


namespace jk_rs485_sniffer {

enum ProtocolVersion {
  PROTOCOL_VERSION_JK04,
  PROTOCOL_VERSION_JK02_24S,
  PROTOCOL_VERSION_JK02_32S,
};

class JkRS485SnifferDevice;

class JkRS485Sniffer : public uart::UARTDevice, public output::TalkPin, public Component {
 public:
  JkRS485Sniffer() = default;


  void set_broadcast_changes_to_all_bms(bool state);
  bool get_broadcast_changes_to_all_bms() const;

  void set_talk_pin(GPIOPin *pin) { talk_pin_ = pin; }
  void set_talk_pin_needed(bool talk_pin_needed) { talk_pin_needed_= talk_pin_needed;}

  void setup() override {

    if (talk_pin_needed_){
      this->turn_off();
      this->talk_pin_->setup();
      this->talk_pin_->digital_write(0);
    }

    for (uint8_t cont=0;cont<16;cont++){
        rs485_network_node[cont].available=0;
        rs485_network_node[cont].last_message_received=0;
        rs485_network_node[cont].last_request_sent=0;
        rs485_network_node[cont].last_device_settings_request_received_OK=0;
        rs485_network_node[cont].last_cell_info_request_received_OK=0;
        rs485_network_node[cont].last_device_info_request_received_OK=0;
        rs485_network_node[cont].counter_cell_info_received=0;
        rs485_network_node[cont].counter_device_settings_received=0;
        rs485_network_node[cont].counter_device_info_received=0;  
    }    
    last_master_activity=0;
    last_message_received_acting_as_master=0;
    last_network_scan=0;

    act_as_master=false;

    pooling_index.node_address=0x0f;
    pooling_index.frame_type=2;
    pooling_index.scan_address=0x01;

    nodes_available_number=0;
    nodes_available.reserve(17);
    for (uint8_t cont = 0; cont < 16; cont++) {
        nodes_available.push_back('0');
    }
    nodes_available.push_back('\0');

    // Reserve upfront so filling rx_buffer_ at runtime doesn't repeatedly
    // reallocate/copy as it grows towards its cap.
    this->rx_buffer_.reserve(RX_BUFFER_MAX_SIZE);
  }

  void loop() override;

  void dump_config() override;

  void register_device(JkRS485SnifferDevice *device) { this->devices_.push_back(device); }

  float get_setup_priority() const override;

  void set_rx_timeout(uint16_t rx_timeout) { rx_timeout_ = rx_timeout; }

  void handle_bms2sniffer_event(std::uint8_t slave_address, std::string event, std::uint8_t frame_type);

  void handle_bms2sniffer_switch_or_number_uint32_event(std::uint8_t slave_address, std::uint8_t third_element_of_frame, std::uint16_t register_address, std::uint32_t value);
  void handle_bms2sniffer_switch_or_number_int32_event(std::uint8_t slave_address, std::uint8_t third_element_of_frame, std::uint16_t register_address, std::int32_t value);
  void handle_bms2sniffer_switch_or_number_uint16_event(std::uint8_t slave_address, std::uint8_t third_element_of_frame,std::uint16_t register_address, std::uint16_t value);
  void send_command_switch_or_number_to_slave_uint32(std::uint8_t slave_address, std::uint8_t third_element_of_frame, std::uint16_t register_address, std::uint32_t value);
  void send_command_switch_or_number_to_slave_int32(std::uint8_t slave_address, std::uint8_t third_element_of_frame, std::uint16_t register_address, std::int32_t value);
  void send_command_switch_or_number_to_slave_uint16(std::uint8_t slave_address, std::uint8_t third_element_of_frame, std::uint16_t register_address, std::uint16_t value);

 protected:
  ProtocolVersion protocol_version_{PROTOCOL_VERSION_JK02_32S};
  
  bool act_as_master;
  uint32_t last_master_activity;
  uint32_t last_message_received_acting_as_master;
  uint32_t last_network_scan;


  void printBuffer(uint16_t max_length);
  uint8_t manage_rx_buffer_(void);
  void set_node_availability(uint8_t address,bool value);
  std::string nodes_available_to_string();

  void detected_master_activity_now(void);
  void send_request_to_slave(uint8_t node_address, uint8_t frame_type);
  
  bool calculate_next_pooling(void);
  // Shared by all three scan orders in calculate_next_pooling(): decides
  // whether `node` is due for a request and, if so, sets pooling_index.frame_type
  // (priority: device info > device settings > cell info). Returns whether
  // anything was due.
  bool decide_next_frame_type(uint8_t node, uint32_t now);
  int found_next_node_to_discover(void);

  // Practical cap on rx_buffer_ growth (see loop()). std::vector::max_size()
  // is the allocator/address-space limit, effectively unbounded in practice,
  // so it can't serve as a growth cap. This is a real ceiling: comfortably
  // more than one full response frame (JKPB_RS485_RESPONSE_SIZE = 308 bytes,
  // defined in the .cpp) plus resync slack, but small enough to bound
  // worst-case memory use if the RS485 line feeds continuous noise that
  // never forms a recognized frame.
  static constexpr size_t RX_BUFFER_MAX_SIZE = 4 * 308;

  std::vector<uint8_t> rx_buffer_;
  uint16_t rx_timeout_{50};
  bool broadcast_changes_to_all_bms_;
  uint32_t last_jk_rs485_network_activity_{0};
  uint32_t last_jk_rs485_pooling_trial_{0};
  uint32_t rx_short_checksum_fail_{0};
  uint32_t rx_request_checksum_fail_{0};
  uint32_t rx_response_checksum_fail_{0};
  uint32_t rx_preamble_drop_{0};
  uint32_t rx_frames_ok_{0};
  std::vector<JkRS485SnifferDevice *> devices_;  

  // talk_pin_ is only meaningful when talk_pin_needed_ is true (real codegen
  // path always calls set_talk_pin_needed() before setup()/loop(), see
  // __init__.py). Guarded here too so this stays safe even if something ever
  // calls write_state() before that, or constructs this class outside codegen.
  void write_state(bool state) override {
    if (this->talk_pin_needed_ && this->talk_pin_ != nullptr) {
      this->talk_pin_->digital_write(state);
    }
  }
  GPIOPin *talk_pin_{nullptr};
  bool talk_pin_needed_{false};

  struct struct_rs485_network_node {
     bool available;
     uint32_t last_message_received;
     uint32_t last_request_sent;
     uint32_t last_device_settings_request_received_OK;
     uint32_t last_device_info_request_received_OK;
     uint32_t last_cell_info_request_received_OK;
     uint16_t counter_cell_info_received;
     uint16_t counter_device_settings_received;
     uint16_t counter_device_info_received;      
  };
  struct struct_rs485_network_node rs485_network_node[16];
  
  std::string nodes_available;
  uint8_t nodes_available_number;

  struct struct_pooling_index {
     uint8_t node_address;
     uint8_t frame_type;
     uint8_t scan_address;
  };

  struct struct_pooling_index pooling_index;
};

class JkRS485SnifferDevice {
 public:
  void set_parent(JkRS485Sniffer *parent) { parent_ = parent; }
  virtual void on_jk_rs485_sniffer_data(const uint8_t &origin_address, const uint8_t &frame_type, const std::vector<uint8_t> &data, const std::string &nodes_available) = 0;

 protected:
  friend JkRS485Sniffer;

  JkRS485Sniffer *parent_{nullptr};
};

}  // namespace jk_rs485_sniffer
}  // namespace esphome
