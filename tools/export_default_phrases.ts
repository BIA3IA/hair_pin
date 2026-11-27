import fs from "node:fs";
import path from "node:path";

// Copia nell'app le frasi del cartello scritte nel firmware (src/cartello/cartello.cpp).
const root = path.resolve(import.meta.dirname, "..");
const source = fs.readFileSync(path.join(root, "src/cartello/cartello.cpp"), "utf8");
const table = source.slice(source.indexOf("const Phrase PHRASES[] = {"), source.indexOf("};", source.indexOf("const Phrase PHRASES[]")));


// categorie solo per l'app: una frase nuova senza categoria finisce in "Altre"
const CATEGORIES: [string, string[]][] = [
  ["Fame e sonno", ["FAME", "CAFFÈ", "Nanna", "Che sonno", "Sono stanca", "Che fatica", "Ancora viva", "SOPRAVVIVO"]],
  ["Confusa", ["ESISTO?", "Dove sono?", "Chi sono?", "EH?", "PERCHÉ?", "Ma quindi?", "Non so", "FORSE?", "FORSE NO?", "BOH", "Che giorno è", "MAH"]],
  ["Distratta", ["Dov'è finito", "Dove ho messo?", "Ho dimenticato qualcosa?", "Ero convinta", "Aspetta un attimo", "Un secondo", "Risolvo dopo"]],
  ["Pensieri e ansia", ["ANSIA", "PANICO", "PENSIERI", "OVER-THINKING", "Quasi bene", "vabbè", "UFFA", "Che sbatti", "Pessima idea", "Scelta dubbia", "Brutta idea"]],
  ["Gatti e misteri", ["RYUK", "Dov'è Ryuk?", "GATTO?", "MIAO", "SPARISCO", "VI OSSERVO", "TI VEDO"]],
  ["Modalità computer", ["GATTO.EXE", "BIANCA.EXE", "System error", "LOADING...", "BUFFERING"]],
  ["Dolce", ["Ti voglio bene", "Che bello", "Giornata top", "EVVIVA", "Coccole?", "Mi manchi"]],
  ["Annoiata", ["Mi annoio", "Uff...", "Mi ignori?", "Sola soletta"]],
];
const phrases: { text: string; when?: string; category: string }[] = [];
for (const match of table.matchAll(/\{"((?:[^"\\]|\\.)*)",\s*(-?\d+),\s*(-?\d+)([^}]*)\}/g)) {
  const [, raw, from, to] = match;
  if (raw === "") continue; // solo icona, senza testo
  const text = raw.replace(/\\n/g, " ").replace(/\\"/g, '"').replace(/- /g, "-");
  const when = from !== "-1" ? `dalle ${from} alle ${to}` : undefined;
  const category = CATEGORIES.find(([, texts]) => texts.includes(text))?.[0] ?? "Altre";
  phrases.push(when ? { text, when, category } : { text, category });
}

// stesso testo in piu' fasce orarie (FAME): una riga sola con entrambe
const groups = [...CATEGORIES.map(([title]) => title), "Altre"]
  .map((title) => {
    const merged = new Map<string, string[]>();
    for (const p of phrases.filter((p) => p.category === title)) {
      const whens = merged.get(p.text) ?? [];
      if (p.when) whens.push(p.when);
      merged.set(p.text, whens);
    }
    return { title, phrases: [...merged].map(([text, whens]) => (whens.length ? { text, when: whens.join(", ") } : { text })) };
  })
  .filter((group) => group.phrases.length > 0);

fs.writeFileSync(
  path.join(root, "app/src/bianca/default-phrases.ts"),
  `// Generato da tools/export_default_phrases.ts: non modificare a mano.
export type DefaultPhrase = { text: string; when?: string };

export const DEFAULT_PHRASE_GROUPS: readonly { title: string; phrases: readonly DefaultPhrase[] }[] = ${JSON.stringify(groups, null, 2)};

export const DEFAULT_PHRASE_COUNT = ${groups.reduce((n, g) => n + g.phrases.length, 0)};
`,
);
console.log(`Esportate ${phrases.length} frasi in ${groups.length} categorie: ${groups.map((g) => `${g.title} ${g.phrases.length}`).join(", ")}`);
