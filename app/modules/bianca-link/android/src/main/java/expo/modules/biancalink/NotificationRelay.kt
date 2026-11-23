package expo.modules.biancalink

import android.content.Context
import android.os.SystemClock

internal object NotificationRelay {
  private val handler = android.os.Handler(android.os.Looper.getMainLooper())
  private val expires = Runnable { pending = null }
  var pending: NotificationItem? = null
    private set
  var lastApp: String? = null
    private set
  fun offer(context: Context, item: NotificationItem) {
    handler.removeCallbacks(expires)
    pending = item
    handler.postDelayed(expires, NotificationPolicy.TTL + 1)
    flush(context)
  }
  fun clear() { handler.removeCallbacks(expires); pending = null }
  fun muteConversation(id: String) { if (pending?.conversationId == id) clear() }
  fun remove(key: String) { if (pending?.key == key) clear() }
  fun flush(context: Context) {
    val item = pending ?: return
    val settings = NotificationSettings(context)
    if (!NotificationPolicy.fresh(item, SystemClock.elapsedRealtime()) || !settings.allowed(item.pkg)) {
      clear(); return
    }
    val mode = settings.rule(item.pkg)?.optString("mode", "app") ?: "app"
    LinkService.instance?.sendNotification(item, NotificationPolicy.packet(item, mode))
  }
  fun delivered(item: NotificationItem) {
    if (pending === item) clear()
    lastApp = item.app
  }
}
