The dials used for to switch menu screens on the display and to switch drive modes both have 12 independent pins with the ability to restrict the range of the dial to less pins. The idea for detecting the position of the dial is to create a custom PCB board that steps the voltage at each dial position and merges to a single output connection. We can then read the voltage passed through the dial using one analog pin.  

ie.  
|Dial position|Output voltage|Menu state|
|-------------|--------------|----------|
|pos 1|1.1v|menu screen 1|
|pos 2|1.6v|menu screen 2|
|pos 3|2.3v|menu screen 3|  



```CPP
int Rotary_output_value;
int dial_steps = 12; // even voltage interval over ## of dial positions
int voltage_bndry = (int)(1024 / dial_steps); // 1024 / 12 = 85.33 (the interval with which the voltage increases -> [i * 85v] )

// loop steps through each interval and checks if the analog voltage read is within the range.
// checking smallest interval to largest allows us to check first if our value is greater than
// a boundary value, if it is, then increases 'i'. If we find our voltage is less than a boundary
// value, then 'i' is our found Rotary_output_value.
for (int i=1; i * voltage_bndry < 1024; i++)
{
  if (AnalogRead(ROTARY_SWITCH_PIN) <= (i * voltage_bndry))
  {
    Rotary_output_value = i;
    break; // we have found and set the value, now exit the loop
  }
}

// now instead, our switch case starts at 1 instead of 0
switch (Rotary_output_value)
{
  case 1:
    // RACE Mode ...

  case 2:
    // ECO Mode ...

  case ##:
    // other subsequent modes ...
}
```
