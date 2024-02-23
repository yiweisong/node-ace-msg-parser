import nodeGypBuild from 'node-gyp-build'
import { join } from 'path'

const binding = nodeGypBuild(join(__dirname, '../')) as any;

const BindingMessageParser = binding.MessageExtractor

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
  messages: MessageFormat[];
}

export interface AllowPacketTypeOptions {
  id: number;
  name: string;
  raw: number[];
}

export interface MessageFormat {
  format: string;
  allowPacketTypes: AllowPacketTypeOptions[];
  skipCheckCRC?: boolean;
}

export interface ResultList {
  packetTypeId: number;
  message: Buffer | string;
  length: number;
  payloadOffset: number;
  payloadLen: number;
}
