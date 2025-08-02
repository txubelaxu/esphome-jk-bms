import esphome.codegen as cg
from esphome.components.number import Number, number_schema, register_number
import esphome.config_validation as cv
from esphome.const import (
    CONF_ID,
    CONF_ICON,
    CONF_UNIT_OF_MEASUREMENT,
    CONF_STEP,
    CONF_MIN_VALUE,
    CONF_MAX_VALUE,
    CONF_MODE,
    CONF_ENTITY_CATEGORY,
    CONF_DEVICE_CLASS,
    DEVICE_CLASS_EMPTY,
    ENTITY_CATEGORY_CONFIG,
    ICON_EMPTY,
    UNIT_VOLT,
    UNIT_AMPERE,
    UNIT_CELSIUS,
    UNIT_EMPTY,
)

from .. import CONF_JK_RS485_BMS_ID, JK_RS485_BMS_COMPONENT_SCHEMA, jk_rs485_bms_ns

DEPENDENCIES = ["jk_rs485_bms"]
CODEOWNERS = ["@syssi", "@txubelaxu"]

JkRS485BmsNumber = jk_rs485_bms_ns.class_("JkRS485BmsNumber", Number, cg.Component)

NUMBER_PARAMS = {
    "cell_smart_sleep_voltage": UNIT_VOLT,
    "cell_undervoltage_protection": UNIT_VOLT,
    "cell_undervoltage_protection_recovery": UNIT_VOLT,
    "cell_overvoltage_protection": UNIT_VOLT,
    "cell_overvoltage_protection_recovery": UNIT_VOLT,
    "cell_balancing_trigger_voltage": UNIT_VOLT,
    "cell_soc100_voltage": UNIT_VOLT,
    "cell_soc0_voltage": UNIT_VOLT,
    "cell_request_charge_voltage": UNIT_VOLT,
    "cell_request_float_voltage": UNIT_VOLT,
    "cell_power_off_voltage": UNIT_VOLT,
    "cell_balancing_starting_voltage": UNIT_VOLT,
    "max_charging_current": UNIT_AMPERE,
    "charging_overcurrent_protection_delay": UNIT_EMPTY,
    "charging_overcurrent_protection_recovery_delay": UNIT_EMPTY,
    "max_discharging_current": UNIT_AMPERE,
    "discharging_overcurrent_protection_delay": UNIT_EMPTY,
    "discharging_overcurrent_protection_recovery_delay": UNIT_EMPTY,
    "short_circuit_protection_delay": UNIT_EMPTY,
    "short_circuit_protection_recovery_delay": UNIT_EMPTY,
    "max_balancing_current": UNIT_AMPERE,
    "charging_overtemperature_protection": UNIT_CELSIUS,
    "charging_overtemperature_protection_recovery": UNIT_CELSIUS,
    "discharging_overtemperature_protection": UNIT_CELSIUS,
    "discharging_overtemperature_protection_recovery": UNIT_CELSIUS,
    "charging_lowtemperature_protection": UNIT_CELSIUS,
    "charging_lowtemperature_protection_recovery": UNIT_CELSIUS,
    "mos_overtemperature_protection": UNIT_CELSIUS,
    "mos_overtemperature_protection_recovery": UNIT_CELSIUS,
    "cell_count_settings": UNIT_EMPTY,
    "battery_capacity_total_settings": UNIT_EMPTY,
    "precharging_time_from_discharge": UNIT_EMPTY,
    "cell_request_charge_voltage_time": UNIT_EMPTY,
    "cell_request_float_voltage_time": UNIT_EMPTY,
}

CONFIG_SCHEMA = JK_RS485_BMS_COMPONENT_SCHEMA.extend({
    cv.Optional(key): number_schema({
        cv.GenerateID(): cv.declare_id(JkRS485BmsNumber),
        cv.Optional(CONF_ICON, default=ICON_EMPTY): cv.icon,
        cv.Optional(CONF_UNIT_OF_MEASUREMENT, default=unit): cv.string_strict,
        cv.Optional(CONF_STEP, default=0.01): cv.float_,
        cv.Optional(CONF_MODE, default="BOX"): cv.string_strict,
        cv.Optional(CONF_ENTITY_CATEGORY, default=ENTITY_CATEGORY_CONFIG): cv.entity_category,
        cv.Optional(CONF_DEVICE_CLASS, default=DEVICE_CLASS_EMPTY): cv.string_strict,
        cv.Optional(CONF_MIN_VALUE, default=0.0): cv.float_,
        cv.Optional(CONF_MAX_VALUE, default=1000.0): cv.float_,
    }).extend(cv.COMPONENT_SCHEMA) for key, unit in NUMBER_PARAMS.items()
})

async def to_code(config):
    hub = await cg.get_variable(config[CONF_JK_RS485_BMS_ID])
    for key in NUMBER_PARAMS:
        if key in config:
            conf = config[key]
            var = cg.new_Pvariable(conf[CONF_ID])
            await cg.register_component(var, conf)
            await register_number(
                var, conf,
                min_value=conf[CONF_MIN_VALUE],
                max_value=conf[CONF_MAX_VALUE],
                step=conf[CONF_STEP],
            )
            cg.add(var.set_parent(hub))
            cg.add(getattr(hub, f"set_{key}_number")(var))
