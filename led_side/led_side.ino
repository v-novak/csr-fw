//#include <rp2040_pio.h>
#include <Adafruit_NeoPixel.h>
#include <Wire.h>

#ifdef __AVR__
#include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

#include "common/comm.h"

const int LED_PIN_LEFT = 2;
const int LED_PIN_RIGHT = 3;
const int LED_COUNT = 22;
Adafruit_NeoPixel strip_left(LED_COUNT, LED_PIN_LEFT, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip_right(LED_COUNT, LED_PIN_RIGHT, NEO_GRB + NEO_KHZ800);

static const uint32_t ORANGE = 0xFF1E00;
static const uint32_t RED    = 0xFF0000;
static const uint32_t GREEN  = 0x00FF00;
static const uint32_t BLUE   = 0x0000FF;
static const uint32_t CYAN   = 0x0080A0;

const uint32_t LED_COLOR_IDLE = CYAN;
const uint32_t LED_COLOR_MOVE = GREEN;

int animation_fps = 30;
led_direction current_dir = LED_DIRECTION_STOP;
int move_pattern[LED_COUNT];
int brightness = 0;
uint32_t animation_step = 0;
int pulse_period = 2;

uint32_t dim(const uint32_t& color, int percent)
{
  uint8_t* color_v = (uint8_t*)&color;
  uint32_t result = color;
  for (int c = 0; c < 4; ++c) {
    uint8_t channel = color_v[c];
    channel = channel * percent / 100;
    ((uint8_t*)&result)[c] = (uint8_t)channel;
  }
  return result;
}

void i2c_handle(int bytes_available)
{
  //Serial.print("recv ");
  //Serial.println(bytes_available);

  I2C_packet input_packet;
  int input_offset = 0;
  for (int i = 0; /*input_offset < sizeof(I2C_packet) && */i < bytes_available; ++input_offset, ++i)
  {
    char c = Wire.read();
    //Serial.print((int)c);
    //Serial.print(" ");
    ((char*)&input_packet)[input_offset] = c;
  }

  if (input_offset < sizeof(I2C_packet)) {
    return;
  }

  input_offset = 0;

  // parse the packet
  switch (input_packet.type) {
    case i2c_packet_command:
        switch (input_packet.command_payload.cmd_type) {
          case i2c_command_led_control:
            current_dir = i2c_led_control_get_direction(input_packet);
            break;
          default:
            break;
        }
      break;

    default:
      // unknown packet type
      break;
  }
}

void setup()
{
#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
#endif

  strip_left.begin();
  strip_left.show();
  strip_left.setBrightness(100);

  strip_right.begin();
  strip_right.show();
  strip_right.setBrightness(100);

  for (byte pix = 0; pix < LED_COUNT; ++pix) {
    move_pattern[pix] = 100 * min(pix, LED_COUNT - pix) / LED_COUNT;
  }

  //pinMode(A4, INPUT_PULLUP);
  //pinMode(A5, INPUT_PULLUP);
  Wire.begin(i2c_addr_led_control);
  Wire.onReceive(i2c_handle); // register event
}


void loop()
{
  strip_left.clear();
  strip_right.clear();

  switch (current_dir) {
  case LED_DIRECTION_STOP:
    for (int c = 0; c < strip_left.numPixels(); ++c) {
      uint32_t color = dim(LED_COLOR_IDLE, move_pattern[(animation_step / 2) % strip_left.numPixels()] / 2 + 20);

      strip_left.setPixelColor(c, color);
      strip_right.setPixelColor(c, color);
    }
    break;
  case LED_DIRECTION_FWD:
    for (int c = 0; c < LED_COUNT; ++c) {
      uint32_t color = dim(LED_COLOR_MOVE, move_pattern[(c + animation_step) % LED_COUNT]);

      strip_left.setPixelColor(c, color);
      strip_right.setPixelColor(c, color);
    }
    break;
  case LED_DIRECTION_BACK:
    for (int c = 0; c < LED_COUNT; ++c) {
      uint32_t color = dim(LED_COLOR_MOVE, move_pattern[(c - animation_step) % LED_COUNT]);

      strip_left.setPixelColor(c, color);
      strip_right.setPixelColor(c, color);
    }
  break;
  case LED_DIRECTION_RIGHT:
  case LED_DIRECTION_LEFT:
    break;
  }

  strip_left.show();                
  strip_right.show();                
  delay(1000 / animation_fps);
  ++animation_step;
}
