#ifndef SERVO_HPP
#define SERVO_HPP

#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "stdio.h"

uint8_t digger_servo_pin = 20;
uint8_t picker_servo_pin = 21;
uint8_t digger_servo_slice;
uint8_t picker_servo_slice;
uint8_t digger_servo_wrap_val;
uint8_t picker_servo_wrap_val;
uint16_t digger_start_duty_ms;
uint16_t digger_end_duty_ms;
uint digger_src_freq = 125000000;
uint digger_servo_freq_ms = 20000;
uint16_t picker_start_duty_ms;
uint16_t picker_end_duty_ms;
uint picker_src_freq = 125000000;
uint picker_servo_freq_ms = 20000;

int map(int x, int x_min, int x_max, int y_min, int y_max)
{
  return (x - x_min) * (y_max - y_min) / (x_max - x_min) + y_min;
}

float degree_to_set_point(const int degree, servo servo)
{
  if (servo == DIGGER)
  {
    int duty_ms = map(degree, 0, 180, digger_start_duty_ms, digger_end_duty_ms);
    // printf("%d: %f\n",duty_ms, float(duty_ms)/float(this->_servo_freq_ms) *1000);
    return float(duty_ms) / float(digger_servo_freq_ms);
  }
  else
  {
    int duty_ms = map(degree, 0, 180, picker_start_duty_ms, picker_end_duty_ms);
    // printf("%d: %f\n",duty_ms, float(duty_ms)/float(this->_servo_freq_ms) *1000);
    return float(duty_ms) / float(picker_servo_freq_ms);
  }
}

void digger_intit()
{
  gpio_set_function(digger_servo_pin, GPIO_FUNC_PWM);
  digger_servo_slice = pwm_gpio_to_slice_num(digger_servo_pin);
  pwm_set_clkdiv(digger_servo_slice, 38.1875f);
  digger_src_freq = 125000000.f / 38.1875f;
  digger_servo_wrap_val = (digger_src_freq / 50) - 1;
  pwm_set_enabled(digger_servo_slice, true);
  pwm_set_wrap(digger_servo_slice, digger_servo_wrap_val);
  pwm_set_gpio_level(digger_servo_pin, 0);
}

void picker_intit()
{
  gpio_set_function(picker_servo_pin, GPIO_FUNC_PWM);
  picker_servo_slice = pwm_gpio_to_slice_num(picker_servo_pin);
  pwm_set_clkdiv(picker_servo_slice, 38.1875f);
  picker_src_freq = 125000000.f / 38.1875f;
  picker_servo_wrap_val = (picker_src_freq / 50) - 1;
  pwm_set_enabled(picker_servo_slice, true);
  pwm_set_wrap(picker_servo_slice, picker_servo_wrap_val);
  pwm_set_gpio_level(picker_servo_pin, 0);
}

void digger_set_position(const uint8_t degree)
{
  float setPoint = degree_to_set_point(degree, DIGGER);
  pwm_set_gpio_level(digger_servo_pin, setPoint);
}

void picker_set_position(const uint8_t degree)
{
  float setPoint = degree_to_set_point(degree, PICKER);
  pwm_set_gpio_level(picker_servo_pin, setPoint);
}

#endif