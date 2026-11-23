import type { AnimationName } from './frames';

export const ANIMATION_LABELS: Record<AnimationName, string> = {
  beat_you: 'Ti sgrida',
  bouncing: 'Saltella',
  coffe: 'Caffè',
  cuddled: 'Coccole',
  dance: 'Danza',
  equilibrio: 'In equilibrio',
  falling_asleep: 'Si addormenta',
  falling_tired: 'Cade stanca',
  get_phone: 'Al telefono',
  grumpy: 'Imbronciata',
  idle_1: 'Tranquilla',
  idle_2: 'In attesa',
  idle_3: 'Si guarda intorno',
  little_jump_in_place: 'Salto felice',
  looking_right: 'Guarda a destra',
  lose_balance: 'Perde l’equilibrio',
  no_head: 'Dice di no',
  pull_card: 'Cartello',
  pull_frame_glow: 'Cornice luminosa',
  pull_orb: 'Sfera',
  push: 'Spinge',
  running: 'Corre',
  sad: 'Triste',
  shake: 'Scossa',
  slide: 'Scivola',
  startled: 'Spavento',
  steady: 'Si sistema',
  strange_pose: 'Posa buffa',
  thoughtful: 'Pensierosa',
  upsidedown: 'A testa in giù',
  walking: 'Cammina',
  wave: 'Saluta',
  yanked: 'Strattonata',
};

export const ANIMATION_NAMES = (Object.keys(ANIMATION_LABELS) as AnimationName[]).sort((a, b) =>
  ANIMATION_LABELS[a].localeCompare(ANIMATION_LABELS[b], 'it'),
);

export type AnimationGroup = 'calm' | 'happy' | 'down' | 'moving';

export const GROUP_TITLES: Record<AnimationGroup, string> = {
  calm: 'Tranquilla',
  happy: 'Felice',
  down: 'Giù di morale',
  moving: 'In movimento',
};

// Record: ogni animazione deve avere un gruppo, altrimenti il typecheck fallisce
const GROUP_OF: Record<AnimationName, AnimationGroup> = {
  idle_1: 'calm',
  idle_2: 'calm',
  idle_3: 'calm',
  steady: 'calm',
  looking_right: 'calm',
  thoughtful: 'calm',
  coffe: 'calm',
  get_phone: 'calm',
  no_head: 'calm',
  strange_pose: 'calm',
  pull_card: 'calm',
  little_jump_in_place: 'happy',
  bouncing: 'happy',
  dance: 'happy',
  wave: 'happy',
  cuddled: 'happy',
  pull_orb: 'happy',
  pull_frame_glow: 'happy',
  sad: 'down',
  grumpy: 'down',
  beat_you: 'down',
  push: 'down',
  falling_tired: 'down',
  falling_asleep: 'down',
  walking: 'moving',
  running: 'moving',
  shake: 'moving',
  upsidedown: 'moving',
  lose_balance: 'moving',
  equilibrio: 'moving',
  slide: 'moving',
  yanked: 'moving',
  startled: 'moving',
};

export const ANIMATION_GROUPS = (Object.keys(GROUP_TITLES) as AnimationGroup[]).map((group) => ({
  group,
  title: GROUP_TITLES[group],
  names: ANIMATION_NAMES.filter((name) => GROUP_OF[name] === group),
}));
