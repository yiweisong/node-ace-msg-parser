const parsers = require("../lib/binding.js");
// const assert = require("assert");

const parser = new parsers.UartMessageParser('parser-key')
console.log(parser)
console.log(parser.receive(Buffer.from([1, 2, 3, 4, 5, 6])));