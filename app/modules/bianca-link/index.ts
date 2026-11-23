import { NativeModule, requireNativeModule } from 'expo';

export type ClipStatus = { mood: number; steps: number; sleeping: boolean; battery: number };
export type LinkState = {
  phase: 'disconnected' | 'pairing' | 'connecting' | 'connected' | 'error';
  message: string;
  status: ClipStatus | null;
};
export type NotificationApp = {
  pkg: string;
  label: string;
  enabled: boolean;
  mode: 'app' | 'sender' | 'preview';
};
export type NotificationState = {
  access: boolean;
  apps: NotificationApp[];
  lastApp: string | null;
  pending: boolean;
};
export type PhraseBundle = { id: string; name: string; phrases: string[] };
// active null = le frasi salvate sulla molletta
export type BundlesState = { active: string | null; bundles: PhraseBundle[] };

declare class BiancaLinkModule extends NativeModule<{ onState: (state: LinkState) => void }> {
  notificationState(): Promise<NotificationState>;
  setNotificationRule(pkg: string, enabled: boolean, mode: string): Promise<void>;
  openNotificationSettings(): Promise<void>;
  getState(): LinkState;
  sendCommands(actions: number[], texts: string[]): Promise<void>;
  connect(): Promise<void>;
  disconnect(): Promise<void>;
  forget(): Promise<void>;
  bundlesState(): Promise<BundlesState>;
  saveBundle(id: string | null, name: string, phrases: string[]): Promise<string>;
  deleteBundle(id: string): Promise<void>;
  setActiveBundle(id: string | null): Promise<void>;
}
export default requireNativeModule<BiancaLinkModule>('BiancaLink');
