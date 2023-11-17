import nodeGypBuild from 'node-gyp-build'
import { join } from 'path'

const binding = nodeGypBuild(join(__dirname, '../')) as any;

const BindingMessageParser = binding.MessageParser

/*~ Write your module's methods and properties in this class */
export class MessageParser {
  private _binding: typeof BindingMessageParser;

  constructor(opts: Options) {
    this._binding = new BindingMessageParser(opts);
  }

  public receive(buffer: Buffer): ResultList[] {
    return this._binding.receive(buffer);
  }
}

/*~ If you want to expose types from your module as well, you can
 *~ place them in this block.
 */

export type PacketLengthType = 'uint8' | 'uint16' | 'uint32'

export interface Options {
  key: string;
  user: UserOptions;
  nmea?: NMEAOptions;
  skipCheckCRC?: boolean;
}

export interface UserOptions {
  allows: number[];
  packetLengthType: PacketLengthType;
}

export interface NMEAOptions {
  allows: string[];
}

export interface ResultList {
  packetType: number;
  payload: Buffer | string;
}
