# MultiDrop Bootloader for AVR

A bootloader for AVR devices on a multidrop bus, like RS485, where all devices can be programmed at once.


## How it works

Somehow the master node will inform the slaves that it is time to be programed.
(most likely by sending a special message on the bus -- this is implementation specific)
The slave nodes will then set the EEPROM value `RUN_APP` to `0` and reset themselves.

Upon reset, the bootloader checks the `RUN_APP` value, and if it is 0, will go
into programming mode.

Master sends a `START` message, to inform the slave nodes that program data is 
about to be sent. Then sends the data, one page at a time. 

Each page will end with a 16 bit CRC to verify the data was received without 
corruption. If the CRC does not match, the slave node will enable the signal
line to inform master of the error.

Master will continue to send all the pages down to the nodes. 

When complete, if the signal line is enabled, it sends the data down again; starting
at the page prior to the signal line becoming enabled.

When all pages have been sent successfully (or max tries has been reached), master
will send a message telling the nodes to run their programs. Now the bootloaders
will set the EEPROM value `RUN_APP` to `1` and reset themselves.

### The Signal Line 

Since we can assume there will be multiple devices listening on the same
communication lines, we cannot have effective 2-way communication between
master and the nodes to be programmed (slave nodes). Instead, the communication
bus needs an additional shared line, called signal, that any slave node can
enable (enable low) to inform the master device of errors.

![Bus Layout](./diagrams/bus.png)

## Messages

Each message follows this format:

|  0     |  1     |  2       |  3           |  4         |  5...n   | n + 1   | n + 2  | 
|--------|--------|----------|--------------|------------|----------|---------|--------|
| `0xFF` | `0xFF` | `<type>` | `<page num>` | `<length>` | `<data>` | `<crc>` | `<crc>`|

The 16-bit CRC value is calculated with every byte after the second starting `0xFF`.

### Start Message

Informs the bootloader that the first page of data is about to be sent.

**Type:** `0xF1`

**Message:** `0xFF 0xFF 0xF1 0 0 0x33 0x20`

| Start       | Type   | Page num | Length | Data | CRC         | 
|-------------|--------|----------|--------|------|-------------|
| `0xFF 0xFF` | `0xF1` | `0x00`   | `0x00` | -    | `0x33 0x20` |


### Page Message

Sends an entire page, or less, of data to the bootloader.

**Type:** `0xF2`

**Example Message:** `0xFF 0xFF 0xF2 0x01 0x05 0x01 0x02 0x03 0x04 0x05 0xB8 0xD0`

| Start       | Type   | Page num | Length | Data                       | CRC         | 
|-------------|--------|----------|--------|----------------------------|-------------|
| `0xFF 0xFF` | `0xF2` | `0x01`   | `0x05` | `0x01 0x02 0x03 0x04 0x05` | `0xB8 0xD0` |

This sends a page number 0x01, which is 5 bytes long,

### End Message

Informs the bootloader that all the data has been sent an it can reset.

**Type:** `0xF3`

**Message:** `0xFF 0xFF 0xF3 0 0 0xF3 0x81`

| Start       | Type   | Page num | Length | Data | CRC         | 
|-------------|--------|----------|--------|------|-------------|
| `0xFF 0xFF` | `0xF3` | `0x00`   | `0x00` | -    | `0xF3 0x81` |

