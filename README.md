# ColourCube Converter
Converts ColourCube resources from Burnout Paradise on Xbox 360 to PC (RGB24).

ColourCubes are a set of 32 32x32 textures used for lighting/tint (may be displayed as one 32x1024 texture). Changing them significantly alters the way the game looks.

There might be an easier way to convert between platforms, I just landed on this implementation by comparing the Xbox and PC resources. Output files should also work for PS4 by moving the data pointer from offset 4 to 8. PS3 and NX support may be added later as it's more or less the same deal, they're all RGB24, just some have their data moved around.