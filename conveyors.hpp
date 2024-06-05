#ifndef CONVERYORS_HPP
#define CONVERYORS_HPP

#include "stdio.h"
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include "enums.hpp"

#define small_con_pulse_pin 10
#define small_con_dir_pin 11

#define large_con_pulse_pin 12
#define large_con_dir_pin 13

uint small_con_clk_src;
uint small_con_pulse_slice;
int32_t small_con_steps_per_rev = 6400;
int32_t small_con_steps_per_sec = 6400;
Speed_Mode small_con_speed_mode = FAST;
uint small_con_wrap_val;
uint small_con_steps_to_take;
int small_con_step_count;
Stepper_Status small_con_status;
Rotation small_con_primary_rotation = CLK;
Rotation small_con_current_rotation;
pwm_config small_con_pwm_config;

uint large_con_clk_src;
uint large_con_pulse_slice;
int32_t large_con_steps_per_rev = 6400;
int32_t large_con_steps_per_sec = 6400;
Speed_Mode large_con_speed_mode = FAST;
uint large_con_wrap_val;
uint large_con_steps_to_take;
int large_con_step_count;
Stepper_Status large_con_status;
Rotation large_con_primary_rotation = CLK;
Rotation large_con_current_rotation;
pwm_config large_con_pwm_config;

void small_con_init(irq_handler_t pwm_handler);
void small_con_pulse_counter();
void small_con_config();
void small_con_rotate(Rotation dir, uint steps);
void small_con_stop();

void large_con_init(irq_handler_t pwm_handler);
void large_con_pulse_counter();
void large_con_config();
void large_con_rotate(Rotation dir, uint steps);
void large_con_stop();

void small_con_init(irq_handler_t pwm_handler)
{
  gpio_init(small_con_dir_pin);
  gpio_set_dir(small_con_dir_pin, GPIO_OUT);

  printf("*-------------------Initializing Small Conveyor-------------------*\n");

  gpio_set_function(small_con_pulse_pin, GPIO_FUNC_PWM);
  small_con_pulse_slice = pwm_gpio_to_slice_num(small_con_pulse_pin);
  pwm_clear_irq(small_con_pulse_slice);
  pwm_set_irq_enabled(small_con_pulse_slice, true);
  irq_set_exclusive_handler(PWM_IRQ_WRAP, pwm_handler);
  irq_set_enabled(PWM_IRQ_WRAP, true);

  small_con_status = STOPPED;
  small_con_step_count = 100000000;
  small_con_steps_to_take = 0;
  printf("PWM Setup Complete\n");
}

void small_con_config()
{

  printf("Configuring PWM Speed\n");
  small_con_pwm_config = pwm_get_default_config();
  switch (small_con_speed_mode)
  {
  case FAST:
    /* NO CLOCK DIVIDER */
    small_con_clk_src = 125000000;
    pwm_config_set_clkdiv(&small_con_pwm_config, 1.f);
    // pwm_set_clkdiv(small_con_pulse_slice, 1.0);
    printf("PWM Speed FAST\n");
    break;
  case MEDIUM:
    /* code */
    pwm_config_set_clkdiv(&small_con_pwm_config, 2.f);
    // pwm_set_clkdiv(small_con_pulse_slice, 2.f);
    small_con_clk_src = 125000000 / 2.f;
    printf("PWM Speed MEDIUM\n");
    break;
  case SLOW:
    /* code */
    pwm_config_set_clkdiv(&small_con_pwm_config, 38.1875f);
    // pwm_set_clkdiv(small_con_clk_src, 38.1875f);
    small_con_clk_src = 125000000 / 38.1875f;
    printf("PWM Speed SLOW\n");
    break;

  default:
    small_con_clk_src = 125000000;
    // pwm_set_clkdiv(small_con_pulse_slice, 1.0);
    break;
  }
  small_con_wrap_val = float(small_con_clk_src) / float(small_con_steps_per_sec) - 1;
  pwm_init(small_con_pulse_slice, &small_con_pwm_config, false);
  pwm_set_wrap(small_con_pulse_slice, small_con_wrap_val);
  printf("%d\n", small_con_wrap_val);
  printf("PWM Configuration Complete\n");
}

void small_con_rotate(Rotation dir, uint steps)
{
  if (small_con_status == RUNNING)
    return;
  printf("Rotating Small Conveyor");
  printf("taking %d steps in %s direction\n", steps, dir == CLK ? "clockwise" : "counter clockwise");
  small_con_status = RUNNING;
  small_con_steps_to_take = steps;
  small_con_current_rotation = dir;
  if (dir == CLK)
  {
    gpio_put(small_con_dir_pin, 0);
  }
  else
  {
    gpio_put(small_con_dir_pin, 1);
  }
  pwm_set_enabled(small_con_pulse_slice, true);
  pwm_set_gpio_level(small_con_pulse_pin, float(small_con_wrap_val) / 2);
}

void small_con_stop()
{
  printf("Stopping\n");
  small_con_status = STOPPED;
  small_con_steps_to_take = 0;
  pwm_set_enabled(small_con_pulse_slice, false);
}

void small_con_pulse_counter()
{
  // printf("%d\n", small_con_steps_to_take);
  pwm_clear_irq(small_con_pulse_slice);
  if (small_con_steps_to_take <= 0 || small_con_step_count <= 0)
    small_con_stop();
  small_con_steps_to_take--;
  if (small_con_current_rotation == small_con_primary_rotation)
  {
    small_con_step_count--;
  }
  else
  {
    small_con_step_count++;
  }
}

void large_con_init(irq_handler_t pwm_handler)
{
  gpio_init(large_con_dir_pin);
  gpio_set_dir(large_con_dir_pin, GPIO_OUT);

  printf("*-------------------Initializing Large Conveyor-------------------*\n");

  gpio_set_function(large_con_pulse_pin, GPIO_FUNC_PWM);
  large_con_pulse_slice = pwm_gpio_to_slice_num(large_con_pulse_pin);
  pwm_clear_irq(large_con_pulse_slice);
  pwm_set_irq_enabled(large_con_pulse_slice, true);
  irq_set_exclusive_handler(PWM_IRQ_WRAP, pwm_handler);
  irq_set_enabled(PWM_IRQ_WRAP, true);

  large_con_status = STOPPED;
  large_con_step_count = 100000000;
  large_con_steps_to_take = 0;
  printf("PWM Setup Complete\n");
}

void large_con_config()
{

  printf("Configuring PWM Speed\n");
  large_con_pwm_config = pwm_get_default_config();
  switch (large_con_speed_mode)
  {
  case FAST:
    /* NO CLOCK DIVIDER */
    large_con_clk_src = 125000000;
    pwm_config_set_clkdiv(&large_con_pwm_config, 1.f);
    // pwm_set_clkdiv(large_con_pulse_slice, 1.0);
    printf("PWM Speed FAST\n");
    break;
  case MEDIUM:
    /* code */
    pwm_config_set_clkdiv(&large_con_pwm_config, 2.f);
    // pwm_set_clkdiv(large_con_pulse_slice, 2.f);
    large_con_clk_src = 125000000 / 2.f;
    printf("PWM Speed MEDIUM\n");
    break;
  case SLOW:
    /* code */
    pwm_config_set_clkdiv(&large_con_pwm_config, 38.1875f);
    // pwm_set_clkdiv(large_con_clk_src, 38.1875f);
    large_con_clk_src = 125000000 / 38.1875f;
    printf("PWM Speed SLOW\n");
    break;

  default:
    large_con_clk_src = 125000000;
    // pwm_set_clkdiv(large_con_pulse_slice, 1.0);
    break;
  }
  large_con_wrap_val = float(large_con_clk_src) / float(large_con_steps_per_sec) - 1;
  pwm_init(large_con_pulse_slice, &large_con_pwm_config, false);
  pwm_set_wrap(large_con_pulse_slice, large_con_wrap_val);
  printf("%d\n", large_con_wrap_val);
  printf("PWM Configuration Complete\n");
}

void large_con_rotate(Rotation dir, uint steps)
{
  if (large_con_status == RUNNING)
    return;
  printf("Rotating Large Conveyor");
  printf("taking %d steps in %s direction\n", steps, dir == CLK ? "clockwise" : "counter clockwise");
  large_con_status = RUNNING;
  large_con_steps_to_take = steps;
  large_con_current_rotation = dir;
  if (dir == CLK)
  {
    gpio_put(large_con_dir_pin, 0);
  }
  else
  {
    gpio_put(large_con_dir_pin, 1);
  }
  pwm_set_enabled(large_con_pulse_slice, true);
  pwm_set_gpio_level(large_con_pulse_pin, float(large_con_wrap_val) / 2);
}

void large_con_stop()
{
  printf("Stopping\n");
  large_con_status = STOPPED;
  large_con_steps_to_take = 0;
  pwm_set_enabled(large_con_pulse_slice, false);
}

void large_con_pulse_counter()
{
  // printf("%d\n", large_con_steps_to_take);
  pwm_clear_irq(large_con_pulse_slice);
  if (large_con_steps_to_take <= 0 || large_con_step_count <= 0)
    large_con_stop();
  large_con_steps_to_take--;
  if (large_con_current_rotation == large_con_primary_rotation)
  {
    large_con_step_count--;
  }
  else
  {
    large_con_step_count++;
  }
}

#endif