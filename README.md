# GD32VF103 Bare-Metal "Hello, World"

This is my first try at writing a minimal bare-metal program for the GD32VF103CB RISC-V microcontroller. These look like fun chips because their peripherals are very similar to those found on the well-understood STM32F103 workhorse, but they have a shiny new RISC-V CPU core which is 50% faster and maybe more power efficient. So while RISC-V is a fairly young architecture, there is still plenty of example code demonstrating how to work with these peripherals, which should make it easier to get started.

It is written for a "Longan Nano" board, which you can buy from Seeed Studios for a little less than $5 each at the time of writing:

[Seeed Studio's Longan Nano Page](https://www.seeedstudio.com/Sipeed-Longan-Nano-RISC-V-GD32VF103CBT6-Development-Board-p-4205.html)

These boards include a common-anode RGB LED, with the cathodes connected to pins `C13` (Red), `A1` (Green), and `A2` (Blue). Since the pins are connected to the cathodes instead of the anodes, writing a `0` turns an LED on and writing a `1` turns it off. You can find more information about how the board is wired in its schematic:

[Longan Nano Schematics](http://dl.sipeed.com/LONGAN/Nano/HDK/Longan%20Nano%202663/Longan%20nano%202663(Schematic).pdf)

# Compiler Toolchain

The GD32VF1 family of chips have a 32-bit RISC-V CPU core with integer multiplication instructions, but no floating-point unit. This is conceptually similar to the ARM Cortex-M3 CPU core which is found in the STM32F1 microcontrollers which...inspired...these chips.

So to build this project, you'll need to build an appropriate GCC toolchain. You can use the RISC-V GCC port configured for an `rv32im` architecture:

[RISC-V GNU toolchain](https://github.com/riscv/riscv-gnu-toolchain)

A basic set of build commands would look something like this:

```
git clone --recursive https://github.com/riscv/riscv-gnu-toolchain
cd riscv-gnu-toolchain
mkdir build
cd build
../configure --with-arch=rv32im --with-prefix=[install directory]
make
```

You might need to install a few dependencies such as the `bison`, `flex`, and `texinfo` packages. If you're missing any, the `make` or `configure` commands will print out errors telling you what to install.

Also, the `--with-prefix` argument is optional; it lets you specity an install directory, like `/opt/riscv-gcc` or `/home/user/riscv-gcc`. This makes it easier to uninstall later, but it also means that you'll need to add `[install directory]/bin` to your `PATH` environment variable in order to run programs like `riscv32-unknown-elf-gcc`.

Finally, this build process will probably take a little bit of time, and it requires a little more than 10GB of free disk space. The `make` command will both build and install the toolchain, so depending on where you want to install it, you might be prompted to run `sudo make`. Once everything is done, you might need to run `sudo ldconfig` or restart your machine before you can run the installed programs normally, but you should be able to delete the build files under your `riscv-gnu-toolchain/build` directory if you need to free up disk space afterwards.

# Project Organization

I think that I might be missing startup code to set up the `ECLIC` interrupt system, but I'm hoping to figure that out by setting up a millisecond 'tick' interrupt to generate the delays between LED toggles.

Some configuration files, such as the OpenOCD files and the RISC-V equivalent of CMSIS headers, are from the GD32VF103 Firmware Library which you can find here:

[GD32VF103 Firmware Library](https://github.com/riscv-mcu/GD32VF103_Firmware_Library)

You can run `openocd -f openocd/openocd_ft2232.cfg` to open a debugging connection using the "Sipeed" USB/JTAG dongles which Seeed Studio sells alongside these boards, but I had to comment out the `ftdi_device_desc` setting to get OpenOCD to recognize mine. You'll also need to use a patched version of OpenOCD which can connect to and flash GD32VF103 chips:

[GD32VF103-compatible RISC-V OpenOCD Fork](https://github.com/riscv-mcu/riscv-openocd)

The `gd32vf103xb_vector_table.S` contains the interrupt vector table, which appears to work similarly to those found in ARM Cortex-M cores. Although, it looks the first entry is supposed to be a "Jump" assembly instruction instead of a memory address to start from.

The actual `reset_handler` function is located in the `src/main.c` file, and it sets up initial register values such as the stack pointer. I've started trying to avoid hand-written assembly files as much as I can, because I think it makes the startup code a little bit easier to read.

The `device_headers/gd32vf103.h` file is hand-written with a few peripheral memory definitions taken from the GD32VF1 reference manual, but it is not comprehensive. I named the definitions to match those found in STM32F1 device header files; the peripherals are very similar, so I'm hoping to start putting together a header file which allows identical driver code to be used for STM32F103 and GD32VF103 chips wherever possible.
