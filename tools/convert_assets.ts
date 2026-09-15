import { readFileSync } from "node:fs";
import path from "node:path";

type Animation = {
  direction: string;
  frames: string[];
};

type SpriteMetadata = {
  states: Array<{
    character: { name: string };
    frames: { animations: Record<string, Animation> };
  }>;
};

const fullPath = path.join("sprites", "metadata.json");
const metadata = JSON.parse(readFileSync(fullPath, "utf8")) as SpriteMetadata;

for (const state of metadata.states) {
  console.log(`Posa: ${state.character.name}`);

  for (const [animationName, animation] of Object.entries(state.frames.animations)) {
    console.log(`  - ${animationName}: ${animation.direction} (${animation.frames.length} frame)`);
  }
}
