#include "stdio.h"
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "axes.hpp"
#include "conveyors.hpp"
#include "servos.hpp"

#define digger_trig 16
#define dropper_trig 17

void trigger_handler(uint pin, uint32_t events) {}

void pwm_handler()
{
  uint32_t pwm_irq_status_mask = pwm_get_irq_status_mask();
  if (pwm_irq_status_mask & (1 << x_axis_pulse_slice))
  {
    x_axis_pulse_counter();
  }
  if (pwm_irq_status_mask & (1 << y_axis_pulse_slice))
  {
    y_axis_pulse_counter();
  }
  if (pwm_irq_status_mask & (1 << small_con_pulse_slice))
  {
    small_con_pulse_counter();
  }
  if (pwm_irq_status_mask & (1 << large_con_pulse_slice))
  {
    large_con_pulse_counter();
  }
}

void switch_init()
{
  gpio_set_irq_enabled_with_callback(digger_trig, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_FALL, true, trigger_handler);
  gpio_set_irq_enabled_with_callback(dropper_trig, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_FALL, true, trigger_handler);
}

int main()
{

  stdio_init_all();
  sleep_ms(5000);
  printf("Starting\n");
  x_axis_steps_per_sec = 6400 * 2;
  x_axis_init(pwm_handler);
  x_axis_config();

  sleep_ms(50);
  y_axis_steps_per_sec = 6400 * 2;
  y_axis_init(pwm_handler);
  y_axis_config();

  sleep_ms(50);

  small_con_init(pwm_handler);
  small_con_config();

  sleep_ms(50);

  large_con_init(pwm_handler);
  large_con_config();

  sleep_ms(50);

  digger_intit();
  picker_intit();

  sleep_ms(50);
  // y_axis_rotate(CLK, 6400 * 1000);

  x_axis_home();
  sleep_ms(1000);
  y_axis_home();
  sleep_ms(1000);

  small_con_rotate(CLK, 6400);
  large_con_rotate(CLK, 6400);

  digger_set_position(0);
  sleep_ms(500);
  digger_set_position(180);
  sleep_ms(500);
  digger_set_position(0);
  sleep_ms(500);

  picker_set_position(0);
  sleep_ms(500);
  picker_set_position(180);
  sleep_ms(500);
  picker_set_position(0);
  sleep_ms(500);

  while (1)
  {
    tight_loop_contents();
  }
}
