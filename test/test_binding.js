const parsers = require("../lib/binding.js");
// const assert = require("assert");

const parser = new parsers.UartMessageParser('parser-key',
    ["s1", "s2", "gN", "iN", "d1", "d2", "sT", "o1", "fM", "rt", "sP"])

const result = parser.receive(Buffer.from([1, 2, 3, 4, 5, 6]));

console.log('result', result);