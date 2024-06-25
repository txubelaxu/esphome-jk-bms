#include "jk_switch.h"
#include "esphome/core/log.h"
#include "esphome/core/application.h"

namespace esphome {
namespace jk_rs485_bms {

static const char *const TAG = "jk_rs485_bms.switch";

void JkRS485BmsSwitch::dump_config() { LOG_SWITCH("", "JkBms Switch", this); }

}  // namespace jk_rs485_bms
}  // namespace esphome