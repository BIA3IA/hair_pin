package expo.modules.biancalink

import android.bluetooth.BluetoothManager
import android.bluetooth.le.ScanFilter
import android.companion.AssociationInfo
import android.companion.AssociationRequest
import android.companion.BluetoothLeDeviceFilter
import android.companion.CompanionDeviceManager
import android.content.Context
import android.content.Intent
import android.content.IntentSender
import android.os.Build
import android.os.Handler
import android.os.Looper
import java.util.Locale
import android.os.ParcelUuid
import expo.modules.kotlin.Promise
import expo.modules.kotlin.functions.Queues
import expo.modules.kotlin.modules.Module
import expo.modules.kotlin.modules.ModuleDefinition

class BiancaLinkModule : Module() {
  private var generation = 0
  private val observer: (Map<String, Any?>) -> Unit = { sendEvent("onState", it) }
  private val context get() = requireNotNull(appContext.reactContext)

  override fun definition() = ModuleDefinition {
    Name("BiancaLink")
    Events("onState")
    OnStartObserving { LinkState.observers.add(observer) }
    OnStopObserving { LinkState.observers.remove(observer) }
    OnDestroy { LinkState.observers.remove(observer) }
    Function("getState") { LinkState.value }

    AsyncFunction("bundlesState") { PhraseBundles(context).state() }
    AsyncFunction("saveBundle") { id: String?, name: String, phrases: List<String> ->
      val bundles = PhraseBundles(context)
      val saved = bundles.save(id, name, phrases)
      if (bundles.active()?.id == saved) LinkService.instance?.applyActiveBundle()
      saved
    }.runOnQueue(Queues.MAIN)
    AsyncFunction("deleteBundle") { id: String ->
      val bundles = PhraseBundles(context)
      val wasActive = bundles.active()?.id == id
      bundles.delete(id)
      if (wasActive) LinkService.instance?.applyActiveBundle()
    }.runOnQueue(Queues.MAIN)
    AsyncFunction("setActiveBundle") { id: String? ->
      PhraseBundles(context).setActive(id)
      LinkService.instance?.applyActiveBundle()
    }.runOnQueue(Queues.MAIN)

    AsyncFunction("notificationState") { NotificationSettings(requireNotNull(appContext.reactContext)).state() }.runOnQueue(Queues.MAIN)
    AsyncFunction("setNotificationRule") { pkg: String, enabled: Boolean, mode: String ->
      val context = requireNotNull(appContext.reactContext)
      NotificationSettings(context).setRule(pkg, enabled, mode)
      NotificationRelay.flush(context)
    }.runOnQueue(Queues.MAIN)
    AsyncFunction("openNotificationSettings") {
      appContext.throwingActivity.startActivity(Intent(android.provider.Settings.ACTION_NOTIFICATION_LISTENER_SETTINGS))
    }.runOnQueue(Queues.MAIN)
    AsyncFunction("connect") {
      val request = ++generation
      val context = requireNotNull(appContext.reactContext)
      val activity = requireNotNull(appContext.currentActivity)
      check(Build.VERSION.SDK_INT >= 33) { "Il collegamento richiede Android 13 o successivo" }
      val bluetooth = context.getSystemService(BluetoothManager::class.java)
      check(bluetooth.adapter?.isEnabled == true) { "Attiva il Bluetooth sul telefono" }
      val saved = context.getSharedPreferences("bianca_link", Context.MODE_PRIVATE).getString("address", null)
      if (saved != null) {
        context.startForegroundService(Intent(context, LinkService::class.java))
      } else {
        val manager = context.getSystemService(CompanionDeviceManager::class.java)
        val filter = BluetoothLeDeviceFilter.Builder().setScanFilter(
          ScanFilter.Builder().setServiceUuid(ParcelUuid(Protocol.service)).build()).build()
        LinkState.update("pairing", "Scegli Bianca e inserisci il codice mostrato sulla molletta")
        manager.associate(AssociationRequest.Builder().addDeviceFilter(filter).build(),
          context.mainExecutor, object : CompanionDeviceManager.Callback() {
            override fun onAssociationPending(sender: IntentSender) {
              if (request != generation) return
              activity.startIntentSenderForResult(sender, 7401, null, 0, 0, 0)
            }
            override fun onAssociationCreated(info: AssociationInfo) {
              Handler(Looper.getMainLooper()).post {
                if (request != generation) return@post
                val address = info.deviceMacAddress?.toString()?.uppercase(Locale.ROOT)
                if (address == null) {
                  LinkState.update("error", "Indirizzo Bluetooth non disponibile")
                } else {
                  context.getSharedPreferences("bianca_link", Context.MODE_PRIVATE).edit()
                    .putString("address", address).putInt("association", info.id).apply()
                  context.startForegroundService(Intent(context, LinkService::class.java))
                }
              }
            }
            override fun onFailure(error: CharSequence?) {
              Handler(Looper.getMainLooper()).post {
                if (request != generation) return@post
                LinkState.update("error", error?.toString() ?: "Abbinamento annullato")
              }
            }
          })
      }
    }.runOnQueue(Queues.MAIN)
    AsyncFunction("sendCommands") { actions: List<Int>, texts: List<String>, promise: Promise ->
      val service = LinkService.instance
      if (service == null) promise.reject("DISCONNECTED", "Collega prima la molletta", null)
      else service.sendCommands(actions, texts, promise)
    }.runOnQueue(Queues.MAIN)
    AsyncFunction("disconnect") {
      generation++
      val context = requireNotNull(appContext.reactContext)
      context.stopService(Intent(context, LinkService::class.java))
      LinkState.update("disconnected", "Molletta scollegata")
    }.runOnQueue(Queues.MAIN)
    AsyncFunction("forget") {
      generation++
      val context = requireNotNull(appContext.reactContext)
      context.stopService(Intent(context, LinkService::class.java))
      val prefs = context.getSharedPreferences("bianca_link", Context.MODE_PRIVATE)
      val id = prefs.getInt("association", -1)
      if (id != -1 && Build.VERSION.SDK_INT >= 33) {
        context.getSystemService(CompanionDeviceManager::class.java).disassociate(id)
      }
      prefs.edit().clear().apply()
      LinkState.update("disconnected", "Molletta rimossa dall’app")
    }.runOnQueue(Queues.MAIN)
  }
}
