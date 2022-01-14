const parsers = require("../lib/binding.js");
const fs = require("fs");
const path = require("path");
// const assert = require("assert");

function convertUint16(packetType) {
    return '0x' + [...packetType].map(item => item.charCodeAt(0).toString(16)).join('');
}

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

// const ethernetParser = new parsers.MessageParser({
//     key: 'ethernet-parser-key',
//     user: {
//         allows: [
//             0x0a01, //raw imu
//             0x0a02, //gnss
//             0x0a03, //ins
//             0x0a04, //odometer
//             0x0a05, //diagnostic
//             0x0a06, //rtcm_rover
//             0x0a07, //misalign
//             0x0a09, //power dr
//             0x4D44, //check
//             0x6749, //gnss integrity
//         ],
//         packetLengthType: 'uint32',
//     },
//     nmea: {
//         allows: allowedNMEATypes,
//     }
// });

let total = 0;

const filePath = path.join(process.cwd(), 'test', 'user.bin');// '/Users/songyiwei/Desktop/debug/20220107/rtk330la_log_2178200286_20220107_145729/user_2022_01_07_14_57_43.bin';
const readStream = fs.createReadStream(filePath);

console.time('uart parser');

// setTimeout(() => {
//     const buf = readStream.read(1000);
//     console.time('parser');
//     const result = parser.receive(buf);
//     //console.log('result', result);
//     total += result.length;
//     console.timeEnd('parser');
// }, 2000)

readStream.on('data', (buf) => {
    const result = uartParser.receive(buf);
    // const existItems = result.filter(t => t.packetType === 'nmea');
    // if (existItems && existItems.length > 0) {
    //     existItems.forEach((item) => {
    //         console.log(String.fromCharCode(...item.payload));
    //     })
    // }
    total += result.length;
})

readStream.on('end', (buf) => {
    console.log('packet amount', total);
    console.timeEnd('uart parser');
})