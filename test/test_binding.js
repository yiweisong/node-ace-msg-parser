const parsers = require("../lib/binding.js");
const fs = require("fs");
const path = require("path");
// const assert = require("assert");

const parser = new parsers.UartMessageParser('parser-key',
    ["s1", "s2", "gN", "iN", "d1", "d2", "sT", "o1", "fM", "rt", "sP"])

let total = 0;

const readStream = fs.createReadStream(path.join(process.cwd(), 'test', 'user.bin'));

console.time('parser');

readStream.on('data', (buf) => {
    const result = parser.receive(buf);
    //console.log('result', result);
    total += result.length;
})

readStream.on('end', (buf) => {
    console.log('packet amount', total);
    console.timeEnd('parser');
})