#ifndef AXES_HPP
#define AXES_HPP

#include "stdio.h"
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include "enums.hpp"

#define x_axis_pulse_pin 2
#define x_axis_dir_pin 4
#define x_axis_limit_switch 5

#define y_axis_pulse_pin 6
#define y_axis_dir_pin 8
#define y_axis_limit_switch 9

uint x_axis_clk_src;
uint x_axis_pulse_slice;
int32_t x_axis_steps_per_rev = 6400;
int32_t x_axis_steps_per_sec = 6400;
Speed_Mode x_axis_speed_mode = FAST;
uint x_axis_wrap_val;
uint x_axis_steps_to_take;
int x_axis_step_count;
Stepper_Status x_status;
Rotation x_axis_primary_rotation = CLK;
Rotation x_axis_current_rotation;
absolute_time_t x_trigger_time;
bool x_axis_limit_reached;
pwm_config x_pwm_config;

uint y_axis_clk_src;
uint y_axis_pulse_slice;
int32_t y_axis_steps_per_rev = 6400;
int32_t y_axis_steps_per_sec = 6400;
Speed_Mode y_axis_speed_mode = FAST;
uint y_axis_wrap_val;
uint y_axis_steps_to_take;
int y_axis_step_count;
Stepper_Status y_status;
Rotation y_axis_primary_rotation = CLK;
Rotation y_axis_current_rotation;
absolute_time_t y_trigger_time;
bool y_axis_limit_reached;
pwm_config y_pwm_config;

void x_axis_init(irq_handler_t pwm_handler);
void x_axis_pulse_counter();
void x_axis_config();
void x_axis_rotate(Rotation dir, uint steps);
void x_axis_stop();
void x_axis_home();

void y_axis_init(irq_handler_t pwm_handler);
void y_axis_pulse_counter();
void y_axis_config();
void y_axis_rotate(Rotation dir, uint steps);
void y_axis_stop();
void y_axis_home();

void x_axis_limit_switch_handler()
{
  // printf("hello\n");
  gpio_acknowledge_irq(x_axis_limit_switch, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE);
  absolute_time_t currTime = get_absolute_time();
  int timeDiff = absolute_time_diff_us(x_trigger_time, currTime) / 1000;
  if (gpio_get_irq_event_mask(x_axis_limit_switch) & GPIO_IRQ_EDGE_FALL && timeDiff > 10 && x_status == RUNNING)
  {
    x_trigger_time = currTime;
    x_axis_limit_reached = true;
    x_axis_stop();
    printf("X-Axis Limit reached\n");
  }
  else if (gpio_get_irq_event_mask(x_axis_limit_switch) & GPIO_IRQ_EDGE_RISE && timeDiff > 100 && x_axis_limit_reached == true)
  {
    x_axis_limit_reached = false;
  }
}

void x_axis_init(irq_handler_t pwm_handler)
{
  gpio_init(x_axis_dir_pin);
  gpio_set_dir(x_axis_dir_pin, GPIO_OUT);

  printf("*-------------------Initializing X-Axis------------------*\n");

  gpio_pull_up(x_axis_limit_switch);
  gpio_set_irq_enabled(x_axis_limit_switch, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true);
  gpio_add_raw_irq_handler(x_axis_limit_switch, x_axis_limit_switch_handler);
  irq_set_enabled(IO_IRQ_BANK0, true);

  printf("Switch Setup Complete\n");

  gpio_set_function(x_axis_pulse_pin, GPIO_FUNC_PWM);
  x_axis_pulse_slice = pwm_gpio_to_slice_num(x_axis_pulse_pin);
  pwm_clear_irq(x_axis_pulse_slice);
  pwm_set_irq_enabled(x_axis_pulse_slice, true);
  irq_set_exclusive_handler(PWM_IRQ_WRAP, pwm_handler);
  irq_set_enabled(PWM_IRQ_WRAP, true);

  x_trigger_time = get_absolute_time();
  x_status = STOPPED;
  x_axis_step_count = 100000000;
  x_axis_steps_to_take = 0;
  x_axis_limit_reached = false;
  printf("PWM Setup Complete\n");
}

void x_axis_config()
{

  printf("Configuring PWM Speed\n");
  x_pwm_config = pwm_get_default_config();
  switch (x_axis_speed_mode)
  {
  case FAST:
    /* NO CLOCK DIVIDER */
    x_axis_clk_src = 125000000;
    pwm_config_set_clkdiv(&x_pwm_config, 1.f);
    // pwm_set_clkdiv(x_axis_pulse_slice, 1.0);
    printf("PWM Speed FAST\n");
    break;
  case MEDIUM:
    /* code */
    pwm_config_set_clkdiv(&x_pwm_config, 2.f);
    // pwm_set_clkdiv(x_axis_pulse_slice, 2.f);
    x_axis_clk_src = 125000000 / 2.f;
    printf("PWM Speed MEDIUM\n");
    break;
  case SLOW:
    /* code */
    pwm_config_set_clkdiv(&x_pwm_config, 38.1875f);
    // pwm_set_clkdiv(x_axis_clk_src, 38.1875f);
    x_axis_clk_src = 125000000 / 38.1875f;
    printf("PWM Speed SLOW\n");
    break;

  default:
    x_axis_clk_src = 125000000;
    // pwm_set_clkdiv(x_axis_pulse_slice, 1.0);
    break;
  }
  x_axis_wrap_val = float(x_axis_clk_src) / float(x_axis_steps_per_sec) - 1;
  pwm_init(x_axis_pulse_slice, &x_pwm_config, false);
  pwm_set_wrap(x_axis_pulse_slice, x_axis_wrap_val);
  printf("%d\n", x_axis_wrap_val);
  printf("PWM Configuration Complete\n");
}

void x_axis_rotate(Rotation dir, uint steps)
{
  if (x_status == RUNNING)
    return;
  printf("Moving X-Axis");
  printf("taking %d steps in %s direction\n", steps, dir == CLK ? "clockwise" : "counter clockwise");
  x_status = RUNNING;
  x_axis_steps_to_take = steps;
  x_axis_current_rotation = dir;
  if (dir == CLK)
  {
    gpio_put(x_axis_dir_pin, 0);
  }
  else
  {
    gpio_put(x_axis_dir_pin, 1);
  }
  pwm_set_enabled(x_axis_pulse_slice, true);
  pwm_set_gpio_level(x_axis_pulse_pin, float(x_axis_wrap_val) / 2);
}

void x_axis_stop()
{
  printf("Stopping\n");
  x_status = STOPPED;
  x_axis_steps_to_take = 0;
  pwm_set_enabled(x_axis_pulse_slice, false);
}

void x_axis_pulse_counter()
{
  // printf("%d\n", x_axis_steps_to_take);
  pwm_clear_irq(x_axis_pulse_slice);

  if (x_axis_steps_to_take <= 0 || x_axis_step_count <= 0)
    x_axis_stop();
  x_axis_steps_to_take--;
  if (x_axis_current_rotation == x_axis_primary_rotation)
  {
    x_axis_step_count--;
  }
  else
  {
    x_axis_step_count++;
  }
}

void x_axis_home()
{
  printf("*---------------Homing X-Axis---------------*\n");
  Rotation opp = x_axis_primary_rotation == CLK ? COUNTER_CLK : CLK;
  // move 1 rotation away from home
  x_axis_rotate(opp, 6400);
  while (x_status == RUNNING)
  {
    sleep_ms(10);
  }
  printf("step 1: status: %d \n", x_status);
  // Start moving towards home untill limit is triggred
  while (!x_axis_limit_reached)
  {
    // if stepper is stopped rotate it once
    if (x_status == STOPPED)
    {
      x_axis_rotate(x_axis_primary_rotation, 6400);
    }
    sleep_ms(10);
  }
  printf("step 2\n");
  // when limit is triggred rotate once away from home
  x_axis_rotate(opp, 6400);
  x_axis_limit_reached = false;
  while (x_status == RUNNING)
  {
    sleep_ms(10);
  }
  printf("step 3\n");
  // set speed to Medium and move towards home untill switch is triggred
  x_axis_speed_mode = MEDIUM;
  x_axis_steps_per_sec = 1500;
  x_axis_config();
  while (!x_axis_limit_reached)
  {
    // if stepper stopped rotate it once
    if (x_status == STOPPED)
    {
      x_axis_rotate(x_axis_primary_rotation, 6400);
    }
    sleep_ms(10);
  }
  // homing done!! set step count to 0 and reset the stepper
  x_axis_step_count = 0;
  x_axis_speed_mode = FAST;
  x_axis_steps_per_sec = 6400;
  x_axis_config();
  printf("step 4! homing done\n");
}

void y_axis_limit_switch_handler()
{
  // printf("hello\n");
  gpio_acknowledge_irq(y_axis_limit_switch, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE);
  absolute_time_t currTime = get_absolute_time();
  int timeDiff = absolute_time_diff_us(y_trigger_time, currTime) / 1000;
  if (gpio_get_irq_event_mask(y_axis_limit_switch) & GPIO_IRQ_EDGE_FALL && timeDiff > 10 && y_status == RUNNING)
  {
    y_trigger_time = currTime;
    y_axis_limit_reached = true;
    y_axis_stop();
    printf("Y-Axis Limit reached\n");
  }
  else if (gpio_get_irq_event_mask(y_axis_limit_switch) & GPIO_IRQ_EDGE_RISE && timeDiff > 100 && y_axis_limit_reached == true)
  {
    y_axis_limit_reached = false;
  }
}

void y_axis_init(irq_handler_t pwm_handler)
{
  gpio_init(y_axis_dir_pin);
  gpio_set_dir(y_axis_dir_pin, GPIO_OUT);

  printf("*-------------------Initializing Y-Axis------------------*\n");

  gpio_pull_up(y_axis_limit_switch);
  gpio_set_irq_enabled(y_axis_limit_switch, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true);
  gpio_add_raw_irq_handler(y_axis_limit_switch, y_axis_limit_switch_handler);
  irq_set_enabled(IO_IRQ_BANK0, true);

  printf("Switch Setup Complete\n");

  gpio_set_function(y_axis_pulse_pin, GPIO_FUNC_PWM);
  y_axis_pulse_slice = pwm_gpio_to_slice_num(y_axis_pulse_pin);
  pwm_clear_irq(y_axis_pulse_slice);
  pwm_set_irq_enabled(y_axis_pulse_slice, true);
  irq_set_exclusive_handler(PWM_IRQ_WRAP, pwm_handler);
  irq_set_enabled(PWM_IRQ_WRAP, true);

  y_trigger_time = get_absolute_time();
  y_status = STOPPED;
  y_axis_step_count = 100000000;
  y_axis_steps_to_take = 0;
  y_axis_limit_reached = false;
  printf("PWM Setup Complete\n");
}

void y_axis_config()
{

  printf("Configuring PWM Speed\n");
  y_pwm_config = pwm_get_default_config();
  switch (y_axis_speed_mode)
  {
  case FAST:
    /* NO CLOCK DIVIDER */
    y_axis_clk_src = 125000000;
    pwm_config_set_clkdiv(&y_pwm_config, 1.f);
    // pwm_set_clkdiv(y_axis_pulse_slice, 1.0);
    printf("PWM Speed FAST\n");
    break;
  case MEDIUM:
    /* code */
    pwm_config_set_clkdiv(&y_pwm_config, 2.f);
    // pwm_set_clkdiv(y_axis_pulse_slice, 2.f);
    y_axis_clk_src = 125000000 / 2.f;
    printf("PWM Speed MEDIUM\n");
    break;
  case SLOW:
    /* code */
    pwm_config_set_clkdiv(&y_pwm_config, 38.1875f);
    // pwm_set_clkdiv(y_axis_clk_src, 38.1875f);
    y_axis_clk_src = 125000000 / 38.1875f;
    printf("PWM Speed SLOW\n");
    break;

  default:
    y_axis_clk_src = 125000000;
    // pwm_set_clkdiv(y_axis_pulse_slice, 1.0);
    break;
  }
  y_axis_wrap_val = float(y_axis_clk_src) / float(y_axis_steps_per_sec) - 1;
  pwm_init(y_axis_pulse_slice, &y_pwm_config, false);
  pwm_set_wrap(y_axis_pulse_slice, y_axis_wrap_val);
  printf("%d\n", y_axis_wrap_val);
  printf("PWM Configuration Complete\n");
}

void y_axis_rotate(Rotation dir, uint steps)
{
  if (y_status == RUNNING)
    return;
  printf("Moving Y-Axis\n");
  y_status = RUNNING;
  y_axis_steps_to_take = steps;
  y_axis_current_rotation = dir;
  printf("taking %d steps in %s direction\n", y_axis_steps_to_take, y_axis_current_rotation == CLK ? "clockwise" : "counter clockwise");
  if (dir == CLK)
  {
    gpio_put(y_axis_dir_pin, 0);
  }
  else
  {
    gpio_put(y_axis_dir_pin, 1);
  }
  pwm_set_enabled(y_axis_pulse_slice, true);
  pwm_set_gpio_level(y_axis_pulse_pin, float(y_axis_wrap_val) / 2);
}

void y_axis_stop()
{
  printf("Stopping\n");
  y_status = STOPPED;
  y_axis_steps_to_take = 0;
  pwm_set_enabled(y_axis_pulse_slice, false);
}

void y_axis_pulse_counter()
{
  // printf("%d\n", y_axis_steps_to_take);
  pwm_clear_irq(y_axis_pulse_slice);
  if (y_axis_steps_to_take <= 0 || y_axis_step_count <= 0)
    y_axis_stop();
  y_axis_steps_to_take--;
  if (y_axis_current_rotation == y_axis_primary_rotation)
  {
    y_axis_step_count--;
  }
  else
  {
    y_axis_step_count++;
  }
}

void y_axis_home()
{
  printf("*---------------Homing Y-Axis---------------*\n");
  Rotation opp = y_axis_primary_rotation == CLK ? COUNTER_CLK : CLK;
  // move 1 rotation away from home
  y_axis_rotate(opp, 6400);
  while (y_status == RUNNING)
  {
    sleep_ms(10);
  }
  printf("step 1: status: %d \n", y_status);
  // Start moving towards home untill limit is triggred
  while (!y_axis_limit_reached)
  {
    // if stepper is stopped rotate it once
    if (y_status == STOPPED)
    {
      y_axis_rotate(y_axis_primary_rotation, 6400);
    }
    sleep_ms(10);
  }
  printf("step 2\n");
  // when limit is triggred rotate once away from home
  y_axis_rotate(opp, 6400);
  y_axis_limit_reached = false;
  while (y_status == RUNNING)
  {
    sleep_ms(10);
  }
  printf("step 3\n");
  // set speed to Medium and move towards home untill switch is triggred
  y_axis_speed_mode = MEDIUM;
  y_axis_steps_per_sec = 1500;
  y_axis_config();
  while (!y_axis_limit_reached)
  {
    // if stepper stopped rotate it once
    if (y_status == STOPPED)
    {
      y_axis_rotate(y_axis_primary_rotation, 6400);
    }
    sleep_ms(10);
  }
  // homing done!! set step count to 0 and reset the stepper
  y_axis_step_count = 0;
  y_axis_speed_mode = FAST;
  y_axis_steps_per_sec = 6400;
  y_axis_config();
  printf("step 4! homing done\n");
}

#endif
