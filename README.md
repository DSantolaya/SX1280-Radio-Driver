# SX1280 LoRa transceiver driver for [contiki-ng](https://github.com/contiki-ng/contiki-ng) OS

This driver was developed to analyze the behaviour of 2.4GHz LoRa radios. 
Includes preliminary protocol stack based on TSCH. To be completed.

Based on git@github.com:tperale/sx128x.git

## How to use

Clone this repository with the `contiki-ng` submodule.

```
git clone --recursive git@github.com:DSantolaya/SX1280-Radio-Driver.git
```

The driver needs to be declare in your `Makefile` by adding
the following line.

```
MODULES += $(CONTIKI_NG_DRIVERS_DIR)/sx128x/src
```

Also be sure to declare the radio driver as your `NETSTACK` radio driver
in the `project-conf.h` file.

```
#define NETSTACK_CONF_RADIO sx128x_radio_driver
```
For energy measurements be sure to declare it.

...
#define ENERGEST_CONF_ON 1
...
