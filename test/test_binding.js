const parsers = require("../dist/index");
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
    messages: [
        {
            format: 'NMEA', allowPacketTypes: [
                { id: 1, name: '$GNGGA', raw: '$GNGGA'.split('').map(t => t.charCodeAt(0)) },
                { id: 2, name: '$GNZDA', raw: '$GNZDA'.split('').map(t => t.charCodeAt(0)) },
                { id: 3, name: '$GPGGA', raw: '$GPGGA'.split('').map(t => t.charCodeAt(0)) },
                { id: 4, name: '$GPZDA', raw: '$GPZDA'.split('').map(t => t.charCodeAt(0)) },
            ]
        },
        {
            format: 'AceinnaBinaryV1', allowPacketTypes: [
                { id: 0x3173, name: 's1', raw: [0x73, 0x31] },
                { id: 0x3273, name: 's2', raw: [0x73, 0x32] },
                { id: 0x4e67, name: 'gN', raw: [0x67, 0x4e] },
                { id: 0x4e69, name: 'iN', raw: [0x69, 0x4e] },
                { id: 0x3164, name: 'd1', raw: [0x64, 0x31] },
                { id: 0x3264, name: 'd2', raw: [0x64, 0x32] },
                { id: 0x5473, name: 'sT', raw: [0x73, 0x54] },
                { id: 0x3169, name: 'o1', raw: [0x69, 0x31] },
                { id: 0x4d66, name: 'fM', raw: [0x66, 0x4d] },
                { id: 0x7472, name: 'rt', raw: [0x72, 0x74] },
                { id: 0x5073, name: 'sP', raw: [0x73, 0x50] },
                { id: 0x3167, name: 'g1', raw: [0x67, 0x31] },
                { id: 0x3169, name: 'i1', raw: [0x69, 0x31] },
                { id: 0x3179, name: 'y1', raw: [0x79, 0x31] },
            ]
        },
    ]
});

const ethernetParser = new parsers.MessageParser({
    key: 'ethernet-parser-key',
    messages:[
        {
            format: 'NMEA', allowPacketTypes: [
                { id: 1, name: '$GNGGA', raw: '$GNGGA'.split('').map(t => t.charCodeAt(0)) },
                { id: 2, name: '$GNZDA', raw: '$GNZDA'.split('').map(t => t.charCodeAt(0)) },
                { id: 3, name: '$GPGGA', raw: '$GPGGA'.split('').map(t => t.charCodeAt(0)) },
                { id: 4, name: '$GPZDA', raw: '$GPZDA'.split('').map(t => t.charCodeAt(0)) },
            ]
        },
        {
            format: 'AceinnaBinaryV2', allowPacketTypes: [
                { id: 0x0a01, name: 'imu', raw: [0x01, 0x0a] },
                { id: 0x0a02, name: 'gnss', raw: [0x02, 0x0a] },
                { id: 0x0a03, name: 'ins', raw: [0x03, 0x0a] },
                { id: 0x0a04, name: 'odo', raw: [0x04, 0x0a] },
                { id: 0x0a05, name: 'diagnostic', raw: [0x05, 0x0a] },
                { id: 0x0a06, name: 'rtcm_rover', raw: [0x06, 0x0a] },
                { id: 0x0a07, name: 'misalign', raw: [0x07, 0x0a] },
                { id: 0x0a09, name: 'power_dr', raw: [0x09, 0x0a] },
                { id: 0x4d44, name: 'check', raw: [0x44, 0x4d] },
                { id: 0x6749, name: 'gi', raw: [0x49, 0x67] },
            ]
        },
    ]
    // user: {
    //     allows: [
    //         0x010a, //raw imu
    //         0x020a, //gnss
    //         0x030a, //ins
    //         0x040a, //odometer
    //         0x050a, //diagnostic
    //         0x060a, //rtcm_rover
    //         0x070a, //misalign
    //         0x090a, //power dr
    //         0x444d, //check
    //         0x4967, //gnss integrity
    //     ],
    //     packetLengthType: 'uint32',
    // },
    // nmea: {
    //     allows: allowedNMEATypes,
    // },
    // skipCheckCRC: true
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
        });

        readStream.on('end', (buf) => {
            console.log('packet amount', total);
            console.timeEnd('uart parser');
            resolve()
        });
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
//testEthernetParser().then(() => { console.log('ethernet parser done') });

