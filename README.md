# Node Aceinna Message Parser

This is a native addon to parse Aceinna device message. It helps you to split the binary data to payload by packet type. Do some decode work based on the user manual if you want to know the actual value from Aceinna device.

> You need to have Node 10.5.0 or later installed. 

## Usage
```javascript
const parsers = require("ace-msg-parser");

const allowedNMEATypes = [
    "$GPGGA", "$GNGGA", "$GPRMC", "$GNRMC", "$GPGSV",
    "$GLGSV", "$GAGSV", "$BDGSV", "$GPGSA", "$GLGSA",
    "$GAGSA", "$BDGSA", "$GPZDA", "$GNZDA", "$GPVTG",
    "$PASHR", "$GNINS",
];

const uartParser = new parsers.MessageParser({
    key: 'uart-parser-key',
    user: {
        allows: [
            0x7331, //s1, 
            0x7332, //s2, 
            0x674e, //gN,
            0x694e, //iN,
            0x6431, //d1,
            0x6432, //d2,
            0x7354, //sT,
            0x6f31, //o1,
            0x664d, //fM,
            0x7274, //rt,
            0x7350, //sP
        ],
        packetLengthType: 'uint8',
    },
    nmea: {
        allows: allowedNMEATypes
    }
});
```

## API
### MessageParser

`constructor(options:Options)`

Accept a options to initialize the parser.

`receive(data: Buffer): ResultList`

Receive some buffer data, and parse the result

### Options
| Name | Data Type | Optional | Description |
| - | - | - | - |
| key | string | No | A unique key|
| user | `UserOptions` | Yes| The options to parse user packet |
| nmea | `NMEAOptions` | Yes | The options to parse NMEA packet |

### UserOptions
| Name | Data Type | Optional | Description |
| - | - | - | - |
| allows | number[] | No | Only data with packet type in the list could be pared |
| packetLengthType | string | No | The packet length type of user packet |

### NMEAOptions
| Name | Data Type | Optional | Description |
| - | - | - | - |
| allows | number[] | No | Only data with NMEA type in the list could be pared |

### ResultList
A list of parsed result
| Name | Data Type | Optional | Description |
| - | - | - | - |
| packetType | number | No | The packet type. `0x6e4d` is for NMEA packet |
| payload | string or Buffer | No | The content of parsed packet. User packet is Buffer, and NMEA is string |