import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import fan, uart
from esphome.const import (
    CONF_OUTPUT_ID,
)
from .. import hubspace_fan_ns

HubspaceFan = hubspace_fan_ns.class_(
    "HubspaceFan", cg.Component, fan.Fan, uart.UARTDevice
)


DEPENDENCIES = ["uart"]

CONFIG_SCHEMA = (
    fan.fan_schema(HubspaceFan).extend({
        cv.GenerateID(CONF_OUTPUT_ID): cv.declare_id(HubspaceFan),
    })
    .extend(cv.COMPONENT_SCHEMA)
    .extend(uart.UART_DEVICE_SCHEMA)
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_OUTPUT_ID])
    await cg.register_component(var, config)
    await fan.register_fan(var, config)
    await uart.register_uart_device(var, config)
