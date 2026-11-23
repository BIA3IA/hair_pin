package expo.modules.biancalink

import androidx.core.app.NotificationCompat
import android.app.Notification
import android.content.ComponentName
import android.os.SystemClock
import android.service.notification.NotificationListenerService
import android.service.notification.StatusBarNotification

class BiancaNotificationListener : NotificationListenerService() {
  private val gate = ConversationGate()
  private val recent = LinkedHashMap<String, Long>()
  override fun onNotificationPosted(sbn: StatusBarNotification) {
    if (!NotificationPolicy.supported(sbn.packageName)) return
    val notification = sbn.notification
    val settings = NotificationSettings(this)
    val label = try {
      packageManager.getApplicationLabel(packageManager.getApplicationInfo(sbn.packageName, 0)).toString()
    } catch (_: android.content.pm.PackageManager.NameNotFoundException) { sbn.packageName }
    settings.seen(sbn.packageName, label)
    val now = SystemClock.elapsedRealtime()
    if (!NotificationPolicy.eligible(settings.allowed(sbn.packageName),
      notification.flags and Notification.FLAG_ONGOING_EVENT != 0,
      notification.flags and Notification.FLAG_GROUP_SUMMARY != 0, null, now)) return
    val title = notification.extras.getCharSequence(Notification.EXTRA_TITLE)?.toString().orEmpty()
    val body = notification.extras.getCharSequence(Notification.EXTRA_TEXT)?.toString().orEmpty()
    val style = NotificationCompat.MessagingStyle.extractMessagingStyleFromNotification(notification)
    val conversation = style?.conversationTitle?.toString()
      ?: notification.extras.getCharSequence(Notification.EXTRA_CONVERSATION_TITLE)?.toString().orEmpty()
    val group = style?.isGroupConversation == true ||
      notification.extras.getBoolean(Notification.EXTRA_IS_GROUP_CONVERSATION, false) || (style == null && conversation.isNotBlank())
    val sender = style?.messages?.lastOrNull()?.person?.name?.toString()?.takeIf { it.isNotBlank() } ?: title
    val chatName = NotificationPolicy.cleanTitle(if (group) conversation else sender, notification.number)
    val chatId = sbn.packageName + ":" + (notification.shortcutId?.takeIf { it.isNotBlank() } ?: chatName.ifBlank { sbn.key })
    val lastMessage = style?.messages?.lastOrNull()
    val messageBody = lastMessage?.text?.toString() ?: body
    val identity = listOf((lastMessage?.timestamp ?: sbn.postTime).toString(), NotificationPolicy.cleanTitle(title), messageBody).joinToString("\u0000")
    val fingerprint = java.security.MessageDigest.getInstance("SHA-256").digest(identity.toByteArray()).joinToString("") { "%02x".format(it) }
    val reportedCount = maxOf(notification.number, style?.messages?.size ?: 0)
    if (!gate.accept(chatId, sbn.key, fingerprint, reportedCount)) {
      if (gate.muted(chatId)) NotificationRelay.muteConversation(chatId)
      return
    }
    if (recent[sbn.key]?.let { now - it < 30000 } == true) return
    if (recent.size >= 256) recent.remove(recent.keys.first())
    recent[sbn.key] = now
    NotificationRelay.offer(this, NotificationItem(sbn.key, sbn.packageName, label,
      NotificationPolicy.truncate(if (group || NotificationPolicy.logoOnly(sbn.packageName)) "" else NotificationPolicy.cleanTitle(sender, notification.number), 192),
      NotificationPolicy.truncate(if (group || NotificationPolicy.logoOnly(sbn.packageName)) "" else messageBody, 192), now, group,
      NotificationPolicy.truncate(NotificationPolicy.cleanTitle(conversation, notification.number), 192), chatId))
  }
  override fun onNotificationRemoved(sbn: StatusBarNotification) {
    gate.remove(sbn.key); recent.remove(sbn.key); NotificationRelay.remove(sbn.key)
  }
  override fun onListenerDisconnected() {
    gate.clear(); recent.clear()
    NotificationRelay.clear()
    requestRebind(ComponentName(this, BiancaNotificationListener::class.java))
  }
}
