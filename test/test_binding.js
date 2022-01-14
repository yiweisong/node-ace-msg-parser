const parsers = require("../lib/binding.js");
const fs = require("fs");
const path = require("path");
// const assert = require("assert");

const parser = new parsers.UartMessageParser('parser-key',
    ["s1", "s2", "gN", "iN", "d1", "d2", "sT", "o1", "fM", "rt", "sP"])

let total = 0;

//
const filePath = path.join(process.cwd(), 'test', 'user.bin');// '/Users/songyiwei/Desktop/debug/20220107/rtk330la_log_2178200286_20220107_145729/user_2022_01_07_14_57_43.bin';
const readStream = fs.createReadStream(filePath);

console.time('parser');

// setTimeout(() => {
//     const buf = readStream.read(1000);
//     console.time('parser');
//     const result = parser.receive(buf);
//     //console.log('result', result);
//     total += result.length;
//     console.timeEnd('parser');
// }, 2000)

readStream.on('data', (buf) => {
    const result = parser.receive(buf);
    // console.log('result', result);
    const existItem = result.find(t => t.packetType === 'nmea');

    total += result.length;
})

readStream.on('end', (buf) => {
    console.log('packet amount', total);
    console.timeEnd('parser');
})