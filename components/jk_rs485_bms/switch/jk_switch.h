#pragma once

#include "../jk_rs485_bms.h"
#include "esphome/core/component.h"
#include "esphome/components/switch/switch.h"

//#include "esphome/core/log.h"


namespace esphome {
namespace jk_rs485_bms {

class JkRS485Bms;  // Declaración anticipada
class JkRS485BmsSwitch : public switch_::Switch, public Component {
 public:
  // Constructor por defecto necesario para crear instancias sin argumentos
  JkRS485BmsSwitch() = default;

  explicit JkRS485BmsSwitch(bool initial_state);
  virtual ~JkRS485BmsSwitch() = default; // Destructor por defecto

  void set_parent(JkRS485Bms *parent) { this->parent_ = parent; };
  void set_register_address(uint16_t register_address) { this->register_address_ = register_address; };
  void set_third_element_of_frame(uint8_t third_element_of_frame) { this->third_element_of_frame_ = third_element_of_frame; };
  void set_data_length(uint8_t data_length) { this->data_length_ = data_length; };
  void dump_config() override;
  void loop() override {}
  float get_setup_priority() const override { return setup_priority::DATA; }

  //void trigger_switch2bms_event(const std::string &event, uint8_t frame_type);

  //void write_state(bool state) override {
  //  this->state = state;
  //  this->publish_state(state);
  //}

  void write_state(bool state) override;
  
 protected:

  // Default member initializers: the explicit JkRS485BmsSwitch(bool) ctor
  // calls write_state() (which reads parent_/data_length_) from its own
  // constructor body, before set_parent()/set_data_length() can ever run.
  // Without these defaults that's a read of indeterminate memory - with
  // them, write_state()'s existing `parent_ == nullptr` guard correctly
  // short-circuits instead. Also protects the no-arg default constructor
  // path used by the real Python codegen (switch/__init__.py), which relies
  // on set_...() being called before any use, but wasn't previously
  // guaranteed at the type level.
  JkRS485Bms *parent_{nullptr};
  uint16_t register_address_{0};
  uint8_t third_element_of_frame_{0};
  uint8_t data_length_{0};
};


}  // namespace jk_rs485_bms
}  // namespace esphome
