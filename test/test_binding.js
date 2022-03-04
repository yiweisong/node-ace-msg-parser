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

            0x6731, //g1
            0x6931, //i1
            0x6f31, //o1
            0x7931, //y1
        ],
        packetLengthType: 'uint8',
    },
    nmea: {
        allows: allowedNMEATypes
    },
    skipCheckCRC: true
});

const ethernetParser = new parsers.MessageParser({
    key: 'ethernet-parser-key',
    user: {
        allows: [
            0x010a, //raw imu
            0x020a, //gnss
            0x030a, //ins
            0x040a, //odometer
            0x050a, //diagnostic
            0x060a, //rtcm_rover
            0x070a, //misalign
            0x090a, //power dr
            0x444d, //check
            0x4967, //gnss integrity
        ],
        packetLengthType: 'uint32',
    },
    nmea: {
        allows: allowedNMEATypes,
    },
    skipCheckCRC: true
});

function testUartParser() {
    return new Promise((resolve) => {
        let total = 0;

        const filePath = '/Users/songyiwei/Desktop/debug/20220107/rtk330la_log_2178200286_20220107_145729/user_2022_01_07_14_57_43.bin'; //path.join(process.cwd(), 'test', 'user.bin');
        const readStream = fs.createReadStream(filePath);

        console.time('uart parser');

        readStream.on('data', (buf) => {
            const result = uartParser.receive(buf);
            total += result.length;
        })

        readStream.on('end', (buf) => {
            console.log('packet amount', total);
            console.timeEnd('uart parser');
            resolve()
        })
    });
}

function testEthernetParser() {
    return new Promise((resolve) => {
        let total = 0;

        // '/Users/songyiwei/Desktop/debug/20220107/rtk330la_log_2178200286_20220107_145729/user_2022_01_07_14_57_43.bin';
        const filePath = "/Users/songyiwei/Desktop/debug/20220107/ins401_log_2179000187_20220107_145728/user_2022_01_07_14_57_36.bin";
        const readStream = fs.createReadStream(filePath);

        console.time('ethernet parser');

        readStream.on('data', (buf) => {
            const result = ethernetParser.receive(buf);
            total += result.length;
        })

        readStream.on('end', (buf) => {
            console.log('packet amount', total);
            console.timeEnd('ethernet parser');
            resolve();
        })
    });
}

// Run sequence
//1. uart parser
//2. ethernet parser
testUartParser().then(() => { return testEthernetParser() });

