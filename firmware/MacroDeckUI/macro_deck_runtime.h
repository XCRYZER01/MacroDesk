#pragma once

/* Mount FAT and replace the built-in profiles when /macrodesk.bin is valid. */
bool macro_deck_runtime_begin(void);

/* Service the MDUP/MDINFO/MDERASE protocol on the USB-to-UART Serial port. */
void macro_deck_runtime_poll(void);
