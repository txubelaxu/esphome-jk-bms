
import esphome.codegen as cg
from esphome.components.text_sensor import TextSensor, text_sensor_schema, register_text_sensor
import esphome.config_validation as cv
from esphome.const import CONF_ICON, CONF_ID, ICON_EMPTY, ICON_TIMELAPSE

from . import CONF_JK_RS485_BMS_ID, JK_RS485_BMS_COMPONENT_SCHEMA

DEPENDENCIES = ["jk_rs485_bms"]
CODEOWNERS = ["@syssi", "@txubelaxu"]

CONF_ERRORS = "errors"
CONF_OPERATION_STATUS = "operation_status"
CONF_BATTERY_TYPE = "battery_type"
CONF_TOTAL_RUNTIME_FORMATTED = "total_runtime_formatted"
CONF_INFO_VENDORID = "info_vendorid"
CONF_INFO_HARDWARE_VERSION = "info_hardware_version"
CONF_INFO_SOFTWARE_VERSION = "info_software_version"
CONF_INFO_DEVICE_NAME = "info_device_name"
CONF_INFO_DEVICE_PASSWORD = "info_device_password"
CONF_NETWORK_NODES_AVAILABLE = "network_nodes_available"
CONF_INFO_DEVICE_SERIAL_NUMBER = "info_device_serial_number"
CONF_INFO_DEVICE_SETUP_PASSCODE = "info_device_setup_passcode"

TEXT_SENSORS = [
    CONF_ERRORS,
    CONF_OPERATION_STATUS,
    CONF_BATTERY_TYPE,
    CONF_TOTAL_RUNTIME_FORMATTED,
    CONF_INFO_VENDORID,
    CONF_INFO_HARDWARE_VERSION,
    CONF_INFO_SOFTWARE_VERSION,
    CONF_INFO_DEVICE_NAME,
    CONF_INFO_DEVICE_PASSWORD,
    CONF_NETWORK_NODES_AVAILABLE,
    CONF_INFO_DEVICE_SERIAL_NUMBER,
    CONF_INFO_DEVICE_SETUP_PASSCODE,
]

CONFIG_SCHEMA = JK_RS485_BMS_COMPONENT_SCHEMA.extend({
    cv.Optional(sensor): text_sensor_schema({
        cv.GenerateID(): cv.declare_id(TextSensor),
        cv.Optional(CONF_ICON, default=ICON_EMPTY): cv.icon,
    }) for sensor in TEXT_SENSORS
})

async def to_code(config):
    hub = await cg.get_variable(config[CONF_JK_RS485_BMS_ID])
    for key in TEXT_SENSORS:
        if key in config:
            conf = config[key]
            sens = cg.new_Pvariable(conf[CONF_ID])
            await register_text_sensor(sens, conf)
            cg.add(sens.set_parent(hub))
            cg.add(getattr(hub, f"set_{key}_text_sensor")(sens))
