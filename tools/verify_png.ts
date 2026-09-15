import { PNG } from "pngjs";
import fs from "node:fs";

const inputFile = process.argv[2] ?? "data/Idle/walking.bin";
const frameIndex = Number(process.argv[3] ?? 0);

const buffer = fs.readFileSync(inputFile);

const width = buffer.readUInt16LE(0);
const height = buffer.readUInt16LE(2);
const frameCount = buffer.readUInt16LE(4);
console.log({ width, height, frameCount });

const maskBytes = Math.ceil((width * height) / 8);
const pixelsBytes = width * height * 2;
const frameBytes = maskBytes + pixelsBytes;

const frameStart = 6 + frameIndex * frameBytes;
const mask = buffer.subarray(frameStart, frameStart + maskBytes);
const pixels = buffer.subarray(frameStart + maskBytes, frameStart + frameBytes);

const png = new PNG({ width, height });

for (let i = 0; i < width * height; i++) {
  const byteIndex = Math.floor(i / 8);
  const bitIndex = i % 8;
  const opaque = (mask[byteIndex] & (1 << bitIndex)) !== 0;

  const rgb565 = pixels.readUInt16LE(i * 2);
  const r5 = (rgb565 >> 11) & 0x1f;
  const g6 = (rgb565 >> 5) & 0x3f;
  const b5 = rgb565 & 0x1f;

  // riporta a 8 bit per canale (approssimato, l'inverso di ">> 3"/">> 2")
  const r8 = (r5 << 3) | (r5 >> 2);
  const g8 = (g6 << 2) | (g6 >> 4);
  const b8 = (b5 << 3) | (b5 >> 2);

  const outIdx = i * 4;
  png.data[outIdx] = r8;
  png.data[outIdx + 1] = g8;
  png.data[outIdx + 2] = b8;
  png.data[outIdx + 3] = opaque ? 255 : 0;
}

const outFile = inputFile.replace(/\.bin$/, `_frame${frameIndex}_decoded.png`);
fs.writeFileSync(outFile, PNG.sync.write(png));
console.log("Scritto", outFile);
