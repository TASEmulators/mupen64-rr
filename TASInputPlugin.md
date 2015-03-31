![http://img219.imageshack.us/img219/9985/tasinputpluginmockupqg0.png](http://img219.imageshack.us/img219/9985/tasinputpluginmockupqg0.png)

# Setting Up #

Place TASDI.dll in the Mupen64 plugins folder. Start Mupen64, select the plugin and configure it. Then load the ROM. If done correctly, there should be a panel for each controller.

If using a joystick, it is necessary to set the Max sensitivity to 127. Uncheck "Pause when not active" in Settings if you don't want the emulator to pause when moving or clicking the panel.

# Usage #

## Analog Stick ##

The picture denotes the offset of the stick from center. Change it by clicking on the picture, or entering numbers in the text boxes, or pressing the keys that control the analog stick.

When pressing the analog stick keys, the X or Y offset changes depending on the settings:

  * Instant: Pressing a direction jumps the offset to an "on" value (determined by the slider) and releasing it resets the offset to 0.
  * Semi-Rel: Holding a direction moves the offset in that direction (speed determined by slider). Pressing in the opposite direction resets the offset before moving again.
  * Relative: Holding a direction moves the offset in that direction (speed determined by slider).
  * Radial: Pressing left/right rotates the offset. Pressing up/down changes the distance from the offset. The distance can be set to Instant, Semi-Rel, or Relative. In Radial mode, sliders change the eccentricity of the rotation.

There are also sliders (increasing to the right) and an angular display box.

## Buttons ##

Left click on a button to autohold. Right click to autofire. Autofire is usually ineffective due to how Mupen64 handles inputs.

## Combos (Macros) ##

Is not supposed to work.

## Other ##

Clicking Hide/More may cause the plugin to stop working.