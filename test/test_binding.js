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

const NMEAFormat = {
    format: 'NMEA', allowPacketTypes: [
        { id: 1, name: '$GNGGA', raw: '$GNGGA'.split('').map(t => t.charCodeAt(0)) },
        { id: 2, name: '$GNZDA', raw: '$GNZDA'.split('').map(t => t.charCodeAt(0)) },
        { id: 3, name: '$GPGGA', raw: '$GPGGA'.split('').map(t => t.charCodeAt(0)) },
        { id: 4, name: '$GPZDA', raw: '$GPZDA'.split('').map(t => t.charCodeAt(0)) },
    ]
};

const RTK330LAUartFormat = {
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
};

const INS401EthernetFormat = {
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
};

const INS502NextFormat = {
    format: 'AceinnaBinaryV3', allowPacketTypes: [
        { id: 0x4e49, name: 'IN', raw: [0x49, 0x4e] },
        { id: 0x3253, name: 'S2', raw: [0x53, 0x32] },
        { id: 0x4e47, name: 'GN', raw: [0x47, 0x4e] },
        { id: 0x5252, name: 'RR', raw: [0x52, 0x52] },
        { id: 0x314f, name: 'ODO', raw: [0x4f, 0x31] },
        { id: 0x4d44, name: 'DIAGNOSTIC', raw: [0x44, 0x4d] },
        { id: 0x5348, name: 'HS', raw: [0x48, 0x53] },
        { id: 0x5249, name: 'IR', raw: [0x49, 0x52] },
        { id: 0x4949, name: 'II', raw: [0x49, 0x49] },
        { id: 0x4947, name: 'GI', raw: [0x47, 0x49] },
    ]
}

const IMU330RA_SFormats = [
    {
        "format": "AceinnaBinaryUdp", "allowPacketTypes": [
            { "id": 12627, "name": "S1", "raw": [83, 49] },
            { "id": 12883, "name": "S2", "raw": [83, 50] },
            { "id": 19782, "name": "FM", "raw": [70, 77] },
            { "id": 19795, "name": "SM", "raw": [83, 77] },
            { "id": 17481, "name": "ID", "raw": [73, 68] },
            { "id": 21078, "name": "VR", "raw": [86, 82] },
            { "id": 19280, "name": "PK", "raw": [80, 75] },
            { "id": 20551, "name": "GP", "raw": [71, 80] },
            { "id": 18007, "name": "WF", "raw": [87, 70] },
            { "id": 18003, "name": "SF", "raw": [83, 70] },
            { "id": 18002, "name": "RF", "raw": [82, 70] },
            { "id": 17991, "name": "GF", "raw": [71, 70] },
            { "id": 18762, "name": "JI", "raw": [74, 73] },
            { "id": 16714, "name": "JA", "raw": [74, 65] },
            { "id": 16727, "name": "WA", "raw": [87, 65] },
            { "id": 21075, "name": "SR", "raw": [83, 82] }
        ],
        "skipCheckCRC": true
    },
    {
        "format": "AceinnaBinaryV2", "allowPacketTypes": [
            { "id": 52225, "name": "ETH_PING", "raw": [1, 204] },
            { "id": 43522, "name": "ETH_JA", "raw": [2, 170] },
            { "id": 43523, "name": "ETH_WA", "raw": [3, 170] },
            { "id": 43524, "name": "ETH_SELECT_CORE", "raw": [4, 170] }],
        "skipCheckCRC": true
    }
]


const buildDataAnalyzer = (key, messageFormats) => {
    return new parsers.MessageParser({
        key,
        messages: messageFormats
    });
}

// const ethernetParser = new parsers.MessageParser({
//     key: 'ethernet-parser-key',
//     messages: [
//         {
//             format: 'NMEA', allowPacketTypes: [
//                 { id: 1, name: '$GNGGA', raw: '$GNGGA'.split('').map(t => t.charCodeAt(0)) },
//                 { id: 2, name: '$GNZDA', raw: '$GNZDA'.split('').map(t => t.charCodeAt(0)) },
//                 //{ id: 3, name: '$GPGGA', raw: '$GPGGA'.split('').map(t => t.charCodeAt(0)) },
//                 //{ id: 4, name: '$GPZDA', raw: '$GPZDA'.split('').map(t => t.charCodeAt(0)) },
//             ]
//         },
//         {
//             format: 'AceinnaBinaryV2', allowPacketTypes: [
//                 { id: 0x0a01, name: 'imu', raw: [0x01, 0x0a] },
//                 { id: 0x0a02, name: 'gnss', raw: [0x02, 0x0a] },
//                 { id: 0x0a03, name: 'ins', raw: [0x03, 0x0a] },
//                 { id: 0x0a04, name: 'odo', raw: [0x04, 0x0a] },
//                 { id: 0x0a05, name: 'diagnostic', raw: [0x05, 0x0a] },
//                 { id: 0x0a06, name: 'rtcm_rover', raw: [0x06, 0x0a] },
//                 { id: 0x0a07, name: 'misalign', raw: [0x07, 0x0a] },
//                 { id: 0x0a09, name: 'power_dr', raw: [0x09, 0x0a] },
//                 { id: 0x4d44, name: 'check', raw: [0x44, 0x4d] },
//                 { id: 0x6749, name: 'gi', raw: [0x49, 0x67] },
//             ]
//         },
//     ]
//     // user: {
//     //     allows: [
//     //         0x010a, //raw imu
//     //         0x020a, //gnss
//     //         0x030a, //ins
//     //         0x040a, //odometer
//     //         0x050a, //diagnostic
//     //         0x060a, //rtcm_rover
//     //         0x070a, //misalign
//     //         0x090a, //power dr
//     //         0x444d, //check
//     //         0x4967, //gnss integrity
//     //     ],
//     //     packetLengthType: 'uint32',
//     // },
//     // nmea: {
//     //     allows: allowedNMEATypes,
//     // },
//     // skipCheckCRC: true
// });

function testAnalyzer(analyzer, filePath, index) {
    return new Promise((resolve) => {
        let total = 0;
        let summary = {};

        //const filePath = '/Users/songyiwei/Desktop/debug/20220107/rtk330la_log_2178200286_20220107_145729/user_2022_01_07_14_57_43.bin'; //path.join(process.cwd(), 'test', 'user.bin');
        const readStream = fs.createReadStream(filePath);

        console.time(`analyzer ${index}`);

        readStream.on('data', (buf) => {
            const result = analyzer.receive(buf);
            result.forEach((item) => {
                if (summary[item.packetTypeId]) {
                    summary[item.packetTypeId] += 1;
                } else {
                    summary[item.packetTypeId] = 1;
                }
            })
            total += result.length;
        });

        readStream.on('end', (buf) => {
            console.log('packet amount', total);
            console.timeEnd(`analyzer ${index}`);
            // console.log('summary');
            Object.keys(summary).forEach((key) => {
                console.log(`0x${Number(key).toString(16)}: ${summary[key]}`);
            });
            resolve()
        });
    });
}

// Run sequence
//1. uart parser
//2. ethernet parser
//testUartParser();//.then(() => { return testEthernetParser() });

//testEthernetParser();
//testEthernetParser().then(() => { console.log('ethernet parser done') });

const analyzers = [];
const rtk330laDataFile = '/Users/songyiwei/Desktop/debug/20220107/rtk330la_log_2178200286_20220107_145729/user_2022_01_07_14_57_43.bin';
const ins401DataFile = '/Users/songyiwei/Desktop/debug/20220107/ins401_log_2179000187_20220107_145728/user_2022_01_07_14_57_36.bin';
const ins502nextDataFile = '/Users/songyiwei/Desktop/transfer/20251017/ins502/user_2025_09_10_13_43_04.bin';
const imu330ra_sDataFile = '/Users/songyiwei/Desktop/transfer/20251017/imu330ra-s/user_2024_11_08_13_46_25.bin';

//const analyzer = buildDataAnalyzer(`ins502-next-parser`, [NMEAFormat, INS502NextFormat]);
//testAnalyzer(analyzer, ins502nextDataFile, 0).then(res => console.log('parser done'));

const analyzer = buildDataAnalyzer(`imu330-ra-parser`, IMU330RA_SFormats);
testAnalyzer(analyzer, imu330ra_sDataFile, 0).then(res => console.log('parser done'));

// for (let i = 0; i < 1; i++) {
//     const analyzer = buildDataAnalyzer(`uart-parser-${i}`, [NMEAFormat, RTK330LAUartFormat]);
//     testAnalyzer(analyzer, rtk330laDataFile, i).then(res=>console.log('uart-parser done'));
// }

// for (let i = 0; i < 1; i++) {
//     const analyzer = buildDataAnalyzer(`ethernet-parser-${i}`, [NMEAFormat, INS401EthernetFormat]);
//     testAnalyzer(analyzer, ins401DataFile, i);
// }
