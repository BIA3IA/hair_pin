package expo.modules.biancalink

// Stato della sessione Android: resta finché esistono notifiche di quella chat.
internal class ConversationGate {
  private data class Chat(var count: Int = 0, val notifications: MutableSet<String> = mutableSetOf())
  private data class Entry(val chat: String, var fingerprint: String)
  private val chats = mutableMapOf<String, Chat>()
  private val entries = mutableMapOf<String, Entry>()
  fun accept(chatId: String, notificationKey: String, fingerprint: String, reportedCount: Int): Boolean {
    val existing = entries[notificationKey]
    if (existing?.chat != null && existing.chat != chatId) remove(notificationKey)
    if (entries[notificationKey]?.fingerprint == fingerprint) return false
    if (chatId !in chats && chats.size >= 256) return false
    if (notificationKey !in entries && entries.size >= 512) return false
    val chat = chats.getOrPut(chatId) { Chat() }
    chat.notifications.add(notificationKey)
    entries[notificationKey] = Entry(chatId, fingerprint)
    chat.count = maxOf((chat.count + 1).coerceAtMost(6), reportedCount.coerceIn(0, 6))
    return chat.count <= 5
  }
  fun muted(chatId: String) = (chats[chatId]?.count ?: 0) > 5
  fun remove(key: String) {
    val entry = entries.remove(key) ?: return
    val chat = chats[entry.chat] ?: return
    chat.notifications.remove(key)
    if (chat.notifications.isEmpty()) chats.remove(entry.chat)
  }
  fun clear() { chats.clear(); entries.clear() }
}
