# ESPHOME-JK-BMS

![GitHub stars](https://img.shields.io/github/stars/rabbit3dcustom/esphome-jk-bms)
![GitHub forks](https://img.shields.io/github/forks/rabbit3dcustom/esphome-jk-bms)
![GitHub watchers](https://img.shields.io/github/watchers/rabbit3dcustom/esphome-jk-bms)

[!["Buy Me A Coffee"](https://img.shields.io/badge/buy%20me%20a%20coffee-donate-yellow.svg)](https://coff.ee/rabbit3dcustom)

## intro
    ESPHome components to monitor Jikong Battery Management Systems (JK-BMS) model PB 15.x via RS-485 with rs485 converter.

## Release versions


- V1.0: https://github.com/rabbit3dcustom/esphome-jk-bms/releases/tag/v.1.0 - Clean non mandatory files.


**NEW:** monitor your new JK-PBx BMSs via internal RS485 network using 1 only ESP. [Click here](https://github.com/rabbit3dcustom/esphome-jk-bms-documentation/tree/main/esphome-jk-bms-component/README.md) for more info. Home Assistant dashboards inside as well.




**Last Change:** Added RCV Time and RFV Time as Read & Write

**Last Change:**
Control your BMSs settings (Numbers):

- cell_smart_sleep_voltage
- cell_undervoltage_protection
- cell_undervoltage_protection_recovery
- cell_overvoltage_protection
- cell_overvoltage_protection_recovery
- cell_balancing_trigger_voltage
- cell_soc100_voltage
- cell_soc0_voltage
- cell_request_charge_voltage
- cell_request_float_voltage
- cell_power_off_voltage
- cell_balancing_starting_voltage
- max_charging_current
- charging_overcurrent_protection_delay
- charging_overcurrent_protection_recovery_delay
- max_discharging_current
- discharging_overcurrent_protection_delay
- discharging_overcurrent_protection_recovery_delay
- short_circuit_protection_delay
- short_circuit_protection_recovery_delay
- max_balancing_current
- charging_overtemperature_protection
- charging_overtemperature_protection_recovery
- discharging_overtemperature_protection
- discharging_overtemperature_protection_recovery
- charging_lowtemperature_protection
- charging_lowtemperature_protection_recovery
- mos_overtemperature_protection
- mos_overtemperature_protection_recovery
- cell_count_settings
- battery_capacity_total_settings
- precharging_time_from_discharge

Broadcast is possible with those settings too.

Updated: 0master_2slave example & 0master_7slave example

Last: control your BMSs settings (Switches) "balancing", "charging" and "discharging" switches. New "broadcast mode" to easy switch on/off every BMS in the network.

### Support

If you want to tip me for this work, you can now buy me a coffee

[!["Buy Me A Coffee"](https://img.shields.io/badge/buy%20me%20a%20coffee-donate-yellow.svg)](https://coff.ee/rabbit3dcustom)

## Old References

(thanks to [@jblance and @jrventer]

- https://www.youtube.com/@OffGridGarageAustralia
- https://github.com/syssi/esphome-jk-bms
- https://github.com/jblance/jkbms
- https://github.com/jblance/mpp-solar/issues/112
- https://github.com/jblance/mpp-solar/blob/master/mppsolar/protocols/jk232.py
- https://github.com/jblance/mpp-solar/blob/master/mppsolar/protocols/jk485.py
- https://github.com/sshoecraft/jktool
- https://github.com/Louisvdw/dbus-serialbattery/blob/master/etc/dbus-serialbattery/jkbms.py
- https://blog.ja-ke.tech/2020/02/07/ltt-power-bms-chinese-protocol.html
