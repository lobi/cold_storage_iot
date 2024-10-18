#ifndef __CONTROLLER_H
#define __CONTROLLER_H
// #include "DataAccess.h"
// #include "utils.h"
// #include "delay.h"


void ctrl_h(char state)
{
  CTRL_H = state == '1' ? 1 : 0;
}

void ctrl_t(char state)
{
  CTRL_T = state == '1' ? 1 : 0;
}

void refresh_ctrl(int hum, int temp)
{
  //int d1_on, d1_off, d2_on, d2_off, wm;
  int i1, i2, i3;

  // wm = DA_GetWorkingMode() * 1;
  // DA_GetHumidity(buf2);
  Delay_ms(ms0);
  if (DA_GetWorkingMode() == '1')
  {
    // control humidifer machine
    Delay_ms(ms0);
    DA_GetDevice2TurnOnAt(buf2);
    i1 = ca2i(buf2, 2);
    Delay_ms(ms0);
    DA_GetDevice2TurnOffAt(buf2);
    i2 = ca2i(buf2, 2);
    if (hum < i1)
    {
      ctrl_h('1');
      // update state
      Delay_ms(ms0);
      DA_SetDevice2State('1');
    }
    else if (hum > i2)
    {
      ctrl_h('0');
      // update state
      Delay_ms(ms0);
      DA_SetDevice2State('0');
    }
    else
    {
      ctrl_h('0');
      // update state
      Delay_ms(ms0);
      DA_SetDevice2State('0');
    }
    

    // control temperature
    Delay_ms(ms0);
    DA_GetDevice1TurnOnAt(buf2);
    i1 = ca2i(buf2, 2);
    Delay_ms(ms0);
    DA_GetDevice1TurnOffAt(buf2);
    i2 = ca2i(buf2, 2);
    
    if (temp > i1)
    {
      ctrl_t('1');
      // update state
      Delay_ms(ms0);
      DA_SetDevice1State('1');
    }
    else if (temp < i2)
    {
      ctrl_t('0');
      // update state
      Delay_ms(ms0);
      DA_SetDevice1State('0');
    }
  }
  else
  {
    // Manual mode
    Delay_ms(ms0);
    ctrl_h(DA_GetDevice2State());
    Delay_ms(ms0);
    ctrl_t(DA_GetDevice1State());
  }
}

#endif