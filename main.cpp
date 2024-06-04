#include "stdio.h"
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "axes.hpp"
#include "conveyors.hpp"

void pwm_handler (){
  uint32_t pwm_irq_status_mask = pwm_get_irq_status_mask();
  if(pwm_irq_status_mask&(1<<x_axis_pulse_slice)){
    x_axis_pulse_counter();
  }
  if(pwm_irq_status_mask&(1<<y_axis_pulse_slice)){
    y_axis_pulse_counter();
  }
  if(pwm_irq_status_mask&(1<<small_con_pulse_slice)){
    small_con_pulse_counter();
  }
  if(pwm_irq_status_mask&(1<<large_con_pulse_slice)){
    large_con_pulse_counter();
  }
}

int main()
{

  stdio_init_all();
  sleep_ms(5000);
  printf("Starting\n");

  x_axis_init(pwm_handler);
  x_axis_config();

  sleep_ms(100);

  y_axis_init(pwm_handler);
  y_axis_config();

  sleep_ms(100);

  small_con_init(pwm_handler);
  small_con_config();

  sleep_ms(100);

  large_con_init(pwm_handler);
  large_con_config();

  sleep_ms(100);

  x_axis_home();
  sleep_ms(1000);
  y_axis_home();
  sleep_ms(1000);

  small_con_rotate(CLK,6400);
  large_con_rotate(CLK,6400);

  while (1)
  {
    tight_loop_contents();
  }
}
