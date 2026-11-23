package expo.modules.biancalink

import android.app.NotificationManager
import android.content.ComponentName
import android.content.Context
import org.json.JSONObject

internal class NotificationSettings(private val context: Context) {
  private val prefs = context.getSharedPreferences("bianca_notifications", Context.MODE_PRIVATE)
  private fun rules() = JSONObject(prefs.getString("apps", "{}")!!)
  fun rule(pkg: String) = rules().optJSONObject(pkg)
  fun seen(pkg: String, label: String) {
    if (!NotificationPolicy.supported(pkg)) return
    val apps = rules()
    val entry = apps.optJSONObject(pkg) ?: JSONObject().put("enabled", false).put("mode", "app")
    entry.put("label", label); apps.put(pkg, entry)
    prefs.edit().putString("apps", apps.toString()).apply()
  }
  fun setRule(pkg: String, enabled: Boolean, mode: String) {
    require(NotificationPolicy.supported(pkg)) { "App non supportata" }
    require(mode in listOf("app", "sender", "preview")) { "Modalità non valida" }
    val apps = rules()
    val entry = apps.optJSONObject(pkg) ?: JSONObject()
    entry.put("enabled", enabled).put("mode", mode); apps.put(pkg, entry)
    prefs.edit().putString("apps", apps.toString()).apply()
  }
  fun allowed(pkg: String) = NotificationPolicy.supported(pkg) && access() && rule(pkg)?.optBoolean("enabled", false) == true
  fun access() = context.getSystemService(NotificationManager::class.java)
    .isNotificationListenerAccessGranted(ComponentName(context, BiancaNotificationListener::class.java))
  // le app supportate installate compaiono subito, senza aspettare una loro notifica
  private fun installedLabel(pkg: String): String? = try {
    val info = context.packageManager.getApplicationInfo(pkg, 0)
    context.packageManager.getApplicationLabel(info).toString()
  } catch (_: android.content.pm.PackageManager.NameNotFoundException) { null }

  fun state(): Map<String, Any?> {
    val apps = rules()
    val listed = NotificationPolicy.packages.mapNotNull { pkg ->
      val label = installedLabel(pkg) ?: apps.optJSONObject(pkg)?.optString("label") ?: return@mapNotNull null
      val entry = apps.optJSONObject(pkg) ?: JSONObject()
      mapOf("pkg" to pkg, "label" to label, "enabled" to entry.optBoolean("enabled", false), "mode" to entry.optString("mode", "app"))
    }
    return mapOf("access" to access(), "apps" to listed.sortedBy { it["label"].toString().lowercase() },
      "lastApp" to NotificationRelay.lastApp, "pending" to (NotificationRelay.pending != null))
  }
}
