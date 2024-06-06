#include "stdio.h"
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "axes.hpp"
#include "conveyors.hpp"
#include "servos.hpp"
#include "tusb.h"
#include <stdlib.h>

#define digger_trig 16
#define dropper_trig 17

char serial_buf[1024];

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

  // x_axis_home();
  // sleep_ms(1000);
  // y_axis_home();
  // sleep_ms(1000);

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

  while (!tud_cdc_connected())
  {
    sleep_ms(10);
  }
  printf("Ready! Enter code\nxa (-/+)steps\nya(-/+)steps\nsc (-/+)\nlc (-/+)\nds degree\nps degree\n");

  while (1)
  {
    char string[1024];
    gets(string);
    char *data = strtok(string, "\n");
    char *cmd = strtok(data, " ");
    char *val = strtok(NULL, " ");
    int value;
    sscanf(val, "%d", &value);
    int cmd_int;
    if (strcmp(cmd, "xa") == 0)
      cmd_int = 0;
    else if (strcmp(cmd, "ya") == 0)
      cmd_int = 1;
    else if (strcmp(cmd, "sc") == 0)
      cmd_int = 2;
    else if (strcmp(cmd, "lc") == 0)
      cmd_int = 3;
    else if (strcmp(cmd, "ds") == 0)
      cmd_int = 4;
    else if (strcmp(cmd, "ps") == 0)
      cmd_int = 5;
    else
      cmd_int = 100;

    switch (cmd_int)
    {
    case 0:
      if (value < 0)
      {
        x_axis_rotate(CLK, abs(value));
      }
      else if (value > 0)
      {
        x_axis_rotate(COUNTER_CLK, value);
      }
      while (x_status == RUNNING)
      {
        sleep_ms(10);
      }
      break;
    case 1:
      if (value < 0)
      {
        y_axis_rotate(CLK, abs(value));
      }
      else if (value > 0)
      {
        y_axis_rotate(COUNTER_CLK, value);
      }
      while (y_status == RUNNING)
      {
        sleep_ms(10);
      }
      break;
    case 2:
      if (value < 0)
      {
        small_con_rotate(CLK, abs(value));
      }
      else if (value > 0)
      {
        small_con_rotate(COUNTER_CLK, value);
      }
      while (small_con_status == RUNNING)
      {
        sleep_ms(10);
      }
      break;
    case 3:
      if (value < 0)
      {
        large_con_rotate(CLK, abs(value));
      }
      else if (value > 0)
      {
        large_con_rotate(COUNTER_CLK, value);
      }
      while (large_con_status == RUNNING)
      {
        sleep_ms(10);
      }
      break;
    case 4:
      digger_set_position(value);
      break;
    case 5:
      picker_set_position(value);
      break;
    default:
      break;
    }
    printf("done\n");
  }
}
