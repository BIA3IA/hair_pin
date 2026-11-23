package expo.modules.biancalink

internal data class NotificationItem(val key: String, val pkg: String, val app: String,
  val title: String, val body: String, val created: Long,
  val isGroup: Boolean = false, val conversationTitle: String = "", val conversationId: String = "")

internal object NotificationPolicy {
  const val TTL = 600000L
  fun truncate(text: String, maxBytes: Int = 48): String {
    val result = StringBuilder()
    var bytes = 0
    val points = text.codePoints().iterator()
    while (points.hasNext()) {
      val point = points.nextInt()
      val part = String(Character.toChars(if (point < 32) 32 else point))
      val size = part.toByteArray(Charsets.UTF_8).size
      if (bytes + size > maxBytes) break
      result.append(part); bytes += size
    }
    return result.toString()
  }
  val packages = setOf("com.whatsapp", "com.whatsapp.w4b", "org.telegram.messenger", "org.telegram.messenger.web", "org.thunderdog.challegram", "com.instagram.android")
  fun supported(pkg: String) = pkg in packages
  fun logoOnly(pkg: String) = pkg == "com.instagram.android"
  fun cleanTitle(title: String, messageCount: Int = 0): String {
    val count = "(?:nuovi? messaggi|messaggi|new messages?|messages?|notifiche|notifications?)"
    val raw = if (messageCount > 1) title.replace(Regex("\\s*[\\(\\[]" + messageCount + "[\\)\\]]\\s*$"), "") else title
    if (Regex("^\\s*\\d+\\s+" + count + "\\s*$", RegexOption.IGNORE_CASE).matches(raw)) return ""
    return raw.replace(Regex("\\s*[\\(\\[]\\s*\\d+\\s+" + count + "\\s*[\\)\\]]\\s*$", RegexOption.IGNORE_CASE), "")
      .replace(Regex("^\\s*\\d+\\s+" + count + "\\s*[:–-]\\s*", RegexOption.IGNORE_CASE), "").trim()
  }
  fun packet(item: NotificationItem, mode: String): ByteArray {
    val text = truncate(if (logoOnly(item.pkg)) "" else if (item.isGroup) {
      cleanTitle(item.conversationTitle).ifBlank { "Gruppo" }
    } else when (mode) {
      "sender" -> cleanTitle(item.title).ifBlank { item.app }
      "preview" -> listOf(cleanTitle(item.title), item.body).filter { it.isNotBlank() }.joinToString(": ").ifBlank { item.app }
      else -> item.app
    })
    val bytes = text.toByteArray(Charsets.UTF_8)
    val icon = when (item.pkg) {
      "com.whatsapp", "com.whatsapp.w4b" -> 9
      "org.telegram.messenger", "org.telegram.messenger.web", "org.thunderdog.challegram" -> 10
      "com.instagram.android" -> 11
      else -> 5
    }
    return byteArrayOf(1, icon.toByte(), 0, bytes.size.toByte()) + bytes
  }
  fun eligible(enabled: Boolean, ongoing: Boolean, summary: Boolean, previous: Long?, now: Long) =
    enabled && !ongoing && !summary && (previous == null || now - previous >= 30000)
  fun fresh(item: NotificationItem, now: Long) = now - item.created in 0..TTL
}
