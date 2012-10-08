# vim:set ft=ioconfig:
# configuration file for io

timers => new System_bus()
{
  WKUPCM => wrap(hw-root.WKUPCM);
  CM2 => wrap(hw-root.CM2);
  GPTIMER1 => wrap(hw-root.GPTIMER1);
  GPTIMER2 => wrap(hw-root.GPTIMER2);
  GPTIMER10 => wrap(hw-root.GPTIMER10);
}

