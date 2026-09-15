import { PNG } from "pngjs";
import fs from "node:fs";
import path from "node:path";

type BBox = { minX: number; minY: number; maxX: number; maxY: number };

// Trova il rettangolo minimo che contiene pixel opachi in una qualsiasi
// delle frame passate (unione su tutti i frame, cosi' il ritaglio resta
// identico per l'intera animazione e il personaggio non "salta").
function computeBoundingBox(pngs: PNG[]): BBox {
  let minX = Infinity, minY = Infinity, maxX = -Infinity, maxY = -Infinity;

  for (const png of pngs) {
    for (let y = 0; y < png.height; y++) {
      for (let x = 0; x < png.width; x++) {
        const alpha = png.data[(png.width * y + x) * 4 + 3];
        if (alpha === 255) {
          if (x < minX) minX = x;
          if (x > maxX) maxX = x;
          if (y < minY) minY = y;
          if (y > maxY) maxY = y;
        }
      }
    }
  }
  return { minX, minY, maxX, maxY };
}

// Legge il canale alpha di ogni pixel DENTRO IL BBOX e lo trasforma in una
// maschera 1-bit: 1 = pixel opaco (va disegnato), 0 = trasparente (salta).
function buildMask(png: PNG, bbox: BBox): Buffer {
  const width = bbox.maxX - bbox.minX + 1;
  const height = bbox.maxY - bbox.minY + 1;
  const maskBytes = Math.ceil((width * height) / 8);
  const mask = Buffer.alloc(maskBytes);

  let i = 0; // indice lineare nell'immagine RITAGLIATA (0..width*height-1)
  for (let y = bbox.minY; y <= bbox.maxY; y++) {
    for (let x = bbox.minX; x <= bbox.maxX; x++) {
      // indice nel PNG originale: usa la larghezza originale, non quella ritagliata
      const alpha = png.data[(png.width * y + x) * 4 + 3];
      if (alpha === 255) {
        const byteIndex = Math.floor(i / 8);
        const bitIndex = i % 8;
        mask[byteIndex] |= 1 << bitIndex;
      }
      i++;
    }
  }
  return mask;
}

// Converte ogni pixel RGB (8 bit per canale) DENTRO IL BBOX in RGB565.
function buildPixels(png: PNG, bbox: BBox): Buffer {
  const width = bbox.maxX - bbox.minX + 1;
  const height = bbox.maxY - bbox.minY + 1;
  const pixels = Buffer.alloc(width * height * 2);

  let i = 0; // indice lineare nell'immagine RITAGLIATA
  for (let y = bbox.minY; y <= bbox.maxY; y++) {
    for (let x = bbox.minX; x <= bbox.maxX; x++) {
      const idx = (png.width * y + x) * 4; // indice nel PNG originale
      const r = png.data[idx];
      const g = png.data[idx + 1];
      const b = png.data[idx + 2];
      const rgb565 = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
      pixels.writeUInt16LE(rgb565, i * 2);
      i++;
    }
  }
  return pixels;
}

// Apre tutti i PNG di un'animazione, nell'ordine dei frame.
function loadAnimationPngs(frameFiles: string[]): PNG[] {
  return frameFiles.map((file) => PNG.sync.read(fs.readFileSync(file)));
}

// Costruisce il file binario di un'intera animazione, ritagliata al bbox
// comune: header (width, height, frameCount) + per ogni frame (maschera + RGB565).
function buildAnimationFile(pngs: PNG[]): Buffer {
  const bbox = computeBoundingBox(pngs);
  const width = bbox.maxX - bbox.minX + 1;
  const height = bbox.maxY - bbox.minY + 1;

  const header = Buffer.alloc(6); // 3 x uint16 = 6 byte
  header.writeUInt16LE(width, 0);
  header.writeUInt16LE(height, 2);
  header.writeUInt16LE(pngs.length, 4);

  const frameBuffers = pngs.map((png) => {
    const mask = buildMask(png, bbox);
    const pixels = buildPixels(png, bbox);
    return Buffer.concat([mask, pixels]);
  });

  return Buffer.concat([header, ...frameBuffers]);
}

// --- ciclo su tutte le pose/animazioni di metadata.json ---
const metadata = JSON.parse(fs.readFileSync("sprites/metadata.json", "utf8"));

for (const state of metadata.states) {
  const poseFolder = state.folder; // es. "Idle", "wooden_frame", ...

  for (const [animationName, animation] of Object.entries(state.frames.animations) as [string, any][]) {
    const framePaths = animation.frames.map((p: string) => path.join("sprites", p));
    const pngs = loadAnimationPngs(framePaths);
    const fileBuffer = buildAnimationFile(pngs);

    const outDir = path.join("data", poseFolder);
    fs.mkdirSync(outDir, { recursive: true });

    const outFile = path.join(outDir, `${animationName}.bin`);
    fs.writeFileSync(outFile, fileBuffer);
    console.log(`Scritto ${outFile}, ${fileBuffer.length} byte`);
  }
}
