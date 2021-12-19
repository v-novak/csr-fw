#ifndef COMM_H
#define COMM_H

enum I2C_addr {
    i2c_addr_motor_board = 9,
    i2c_addr_led_control = 10
};

enum I2C_packet_type {
    i2c_packet_command = 0
};

enum I2C_command_type {
    i2c_command_wheel_control = 0,
    i2c_command_led_control = 1
};

const unsigned i2c_max_payload = 16; // 16 bytes should be enough...

struct I2C_command {
    I2C_command_type cmd_type:8;
    unsigned char cmd_args[i2c_max_payload - sizeof(I2C_command_type)];
};

struct I2C_packet
{
    I2C_packet_type type:8;
    union {
        unsigned char payload[i2c_max_payload];
        I2C_command command_payload;
    };
};

enum led_direction {
    LED_DIRECTION_STOP = 0,
    LED_DIRECTION_FWD = 1,
    LED_DIRECTION_BACK = 2,
    LED_DIRECTION_RIGHT = 3,
    LED_DIRECTION_LEFT = 4
};

led_direction i2c_led_control_get_direction(const I2C_packet& cmd)
{
    return cmd.command_payload.cmd_args[0];
}

void i2c_led_control_set_direction(I2C_packet& cmd, led_direction direction)
{
    cmd.command_payload.cmd_args[0] = direction;
}

#endif // COMM_H
