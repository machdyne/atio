# ATIO

ATIO is RP2040 firmware that lets you control IO with an AT command set.

## Usage

Plug the device into a computer.

```
$ minicom -D /dev/ttyACM0 -b 115200
```

## Building

To build the RP2040 firmware, assuming you have [pico-sdk](https://github.com/raspberrypi/pico-sdk) installed:

```
$ mkdir build
$ cd build
$ cmake ..
$ make
```

To build and run a simulated version on Linux for debugging/testing:

```
$ make
$ ./atio
```

## Protocol

ATIO received commands can be terminated with CR, LF or CRLF.

ATIO transmitted messages are terminated with CRLF.

ATIO sends informational messages prefixed with "#".

When ATIO is initialized it will send "READY".

ATIO commands respond with "OK", "ERROR" or "+value".

### Test

```
> AT
< OK
```

### Reset device

```
> AT+RST
< ...
< READY
```

### Set direction of GPIO n (0 = input, 1 = output)

```
> AT+GPIODn=X
< OK
```

Example:

```
> AT+GPIO5=1
```

### Enable pull-up resistor for GPIO n

```
> AT+GPIOPUn
< OK
```

### Enable pull-down resistor for GPIO n

```
> AT+GPIOPDn
< OK
```

### Disable pull-up/pull-down resistors for GPIO n

```
> AT+GPIOPNn
< OK
```

### Set value for GPIO n

```
> AT+GPIOn=1
< OK
```

### Get value for GPIO n

```
> AT+GPIOn?
< +1
```

## License

ATIO is released under the Lone Dynamics Open License.
