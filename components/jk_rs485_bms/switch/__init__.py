
import esphome.codegen as cg
from esphome.components.switch import Switch, switch_schema, register_switch
import esphome.config_validation as cv
from esphome.const import CONF_ICON, CONF_ID

from .. import CONF_JK_RS485_BMS_ID, JK_RS485_BMS_COMPONENT_SCHEMA, jk_rs485_bms_ns
from ..const import *

DEPENDENCIES = ["jk_rs485_bms"]
CODEOWNERS = ["@syssi", "@txubelaxu"]

ICON_CHARGING = "mdi:battery-charging-50"
ICON_DISCHARGING = "mdi:battery-charging-50"
ICON_BALANCING = "mdi:seesaw"
ICON_EMERGENCY = "mdi:exit-run"
ICON_HEATING = "mdi:radiator"
ICON_DISABLE_TEMPERATURE_SENSORS = "mdi:thermometer-off"
ICON_SMART_SLEEP_ON = "mdi:sleep"
ICON_TIMED_STORED_DATA = "mdi:calendar-clock"
ICON_DISABLE_PCL_MODULE = "mdi:power-plug-off"
ICON_CHARGING_FLOAT_MODE = "mdi:battery-charging-80"
ICON_DISPLAY_ALWAYS_ON = "mdi:television"

SWITCHES = {
    CONF_CHARGING: [0x0070, 0x10, 0x04],
    CONF_DISCHARGING: [0x0074, 0x10, 0x04],
    CONF_BALANCING: [0x0078, 0x10, 0x04],
    CONF_HEATING: [0x0014, 0x11, 0x02],
    CONF_DISABLE_TEMPERATURE_SENSORS: [0x0014, 0x11, 0x02],
    CONF_GPS_HEARTBEAT: [0x0014, 0x11, 0x02],
    CONF_PORT_SELECTION: [0x0014, 0x11, 0x02],
    CONF_DISPLAY_ALWAYS_ON: [0x0014, 0x11, 0x02],
    CONF_SPECIAL_CHARGER: [0x0014, 0x11, 0x02],
    CONF_SMART_SLEEP_ON: [0x0014, 0x11, 0x02],
    CONF_DISABLE_PCL_MODULE: [0x0014, 0x11, 0x02],
    CONF_TIMED_STORED_DATA: [0x0014, 0x11, 0x02],
    CONF_CHARGING_FLOAT_MODE: [0x0014, 0x11, 0x02],
    CONF_PRECHARGING: [0x0000, 0x00, 0x00],
    CONF_EMERGENCY: [0x0000, 0x00, 0x00],
}

JkRS485BmsSwitch = jk_rs485_bms_ns.class_("JkRS485BmsSwitch", Switch, cg.Component)

def make_schema(conf, icon):
    return switch_schema({
        cv.GenerateID(): cv.declare_id(JkRS485BmsSwitch),
        cv.Optional(CONF_ICON, default=icon): cv.icon,
    }).extend(cv.COMPONENT_SCHEMA)

CONFIG_SCHEMA = JK_RS485_BMS_COMPONENT_SCHEMA.extend({
    cv.Optional(k): make_schema(k, icon) for k, icon in {
        CONF_PRECHARGING: ICON_CHARGING,
        CONF_CHARGING: ICON_CHARGING,
        CONF_DISCHARGING: ICON_DISCHARGING,
        CONF_BALANCING: ICON_BALANCING,
        CONF_EMERGENCY: ICON_EMERGENCY,
        CONF_HEATING: ICON_HEATING,
        CONF_DISABLE_TEMPERATURE_SENSORS: ICON_DISABLE_TEMPERATURE_SENSORS,
        CONF_DISPLAY_ALWAYS_ON: ICON_DISPLAY_ALWAYS_ON,
        CONF_SMART_SLEEP_ON: ICON_SMART_SLEEP_ON,
        CONF_TIMED_STORED_DATA: ICON_TIMED_STORED_DATA,
        CONF_CHARGING_FLOAT_MODE: ICON_CHARGING_FLOAT_MODE,
        CONF_DISABLE_PCL_MODULE: ICON_DISABLE_PCL_MODULE,
        CONF_GPS_HEARTBEAT: ICON_DISABLE_PCL_MODULE,
        CONF_PORT_SELECTION: ICON_DISABLE_PCL_MODULE,
        CONF_SPECIAL_CHARGER: ICON_DISABLE_PCL_MODULE,
    }.items()
})

async def to_code(config):
    hub = await cg.get_variable(config[CONF_JK_RS485_BMS_ID])
    for key, param_config in SWITCHES.items():
        if key in config:
            conf = config[key]
            var = cg.new_Pvariable(conf[CONF_ID])
            await cg.register_component(var, conf)
            await register_switch(var, conf)
            cg.add(getattr(hub, f"set_{key}_switch")(var))
            cg.add(var.set_parent(hub))
            cg.add(var.set_register_address(param_config[0]))
            cg.add(var.set_third_element_of_frame(param_config[1]))
            cg.add(var.set_data_length(param_config[2]))
