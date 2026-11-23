package expo.modules.biancalink

import android.Manifest
import android.app.*
import android.bluetooth.*
import android.content.*
import android.content.pm.PackageManager
import android.os.*
import java.util.Locale
import expo.modules.kotlin.Promise

class LinkService : Service() {
  companion object { var instance: LinkService? = null; private set }

  // un gruppo di pacchetti si scrive tutto prima del successivo
  private class Batch(val packets: List<ByteArray>, val done: (String?) -> Unit)
  private val batches = java.util.ArrayDeque<Batch>()
  private var batch: Batch? = null
  private val commands = java.util.ArrayDeque<ByteArray>()
  private var notificationInFlight: NotificationItem? = null
  private val commandTimeout = Runnable { failed("Invio del comando scaduto") }

  private val connected get() = LinkState.value["phase"] == "connected" && gatt != null

  private fun enqueue(packets: List<ByteArray>, done: (String?) -> Unit) {
    if (!connected) { done("Collega prima la molletta"); return }
    batches.add(Batch(packets, done))
    pump()
  }

  private fun pump() {
    if (batch != null) return
    val next = batches.pollFirst() ?: return
    batch = next
    commands.addAll(next.packets)
    handler.postDelayed(commandTimeout, 15000L + 200L * next.packets.size)
    writeNext()
  }

  private fun finishBatch(error: String?) {
    handler.removeCallbacks(commandTimeout)
    val finished = batch ?: return
    batch = null
    finished.done(error)
    if (error == null) {
      NotificationRelay.flush(this)
      pump()
    }
  }

  internal fun sendNotification(item: NotificationItem, packet: ByteArray) {
    if (!connected || notificationInFlight != null) return
    notificationInFlight = item
    enqueue(listOf(packet)) { error ->
      if (error == null) NotificationRelay.delivered(item)
      notificationInFlight = null
    }
  }

  fun sendCommands(actions: List<Int>, texts: List<String>, promise: Promise) {
    require(actions.size == texts.size && actions.size in 1..BundleRules.MAX_PHRASES + 2) { "Numero di comandi non valido" }
    val packets = actions.zip(texts).map { (action, text) -> Protocol.command(action, text) }
    enqueue(packets) { error -> if (error == null) promise.resolve(null) else promise.reject("LINK_ERROR", error, null) }
  }

  // il bundle scelto nell'app sostituisce le frasi della molletta finche' e' collegata
  fun applyActiveBundle() {
    if (!connected) return
    enqueue(Protocol.bundle(PhraseBundles(this).active()?.phrases)) {}
  }

  private fun writeNext() {
    if (commands.isEmpty()) {
      finishBatch(null)
      return
    }
    val current = gatt
    val characteristic = current?.getService(Protocol.service)?.getCharacteristic(Protocol.notify)
    if (current == null || characteristic == null || current.writeCharacteristic(characteristic,
        commands.removeFirst(), BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT) != BluetoothStatusCodes.SUCCESS) {
      failed("Invio del comando non riuscito")
    }
  }

  private val handler = Handler(Looper.getMainLooper())
  private var gatt: BluetoothGatt? = null
  private var stopped = false
  private var attempt = 0
  private val retry = Runnable { connect() }
  private val timeout = Runnable { failed("Connessione scaduta: controlla il codice di abbinamento") }
  private val bluetoothReceiver = object : BroadcastReceiver() {
    override fun onReceive(context: Context, intent: Intent) {
      when (intent.getIntExtra(BluetoothAdapter.EXTRA_STATE, -1)) {
        BluetoothAdapter.STATE_OFF -> failed("Bluetooth spento: riattivalo sul telefono")
        BluetoothAdapter.STATE_ON -> { handler.removeCallbacks(retry); connect() }
      }
    }
  }

  override fun onCreate() {
    super.onCreate()
    instance = this
    val manager = getSystemService(NotificationManager::class.java)
    manager.createNotificationChannel(NotificationChannel("bianca_link", "Collegamento Bianca", NotificationManager.IMPORTANCE_LOW))
    val launch = packageManager.getLaunchIntentForPackage(packageName)
    val notification = Notification.Builder(this, "bianca_link")
      .setSmallIcon(android.R.drawable.stat_sys_data_bluetooth)
      .setContentTitle("Bianca").setContentText("Collegamento alla molletta attivo")
      .setOngoing(true)
    if (launch != null) notification.setContentIntent(PendingIntent.getActivity(this, 0, launch, PendingIntent.FLAG_IMMUTABLE))
    startForeground(7402, notification.build())
    registerReceiver(bluetoothReceiver, IntentFilter(BluetoothAdapter.ACTION_STATE_CHANGED))
  }

  override fun onStartCommand(intent: Intent?, flags: Int, startId: Int): Int {
    if (gatt == null) connect()
    return START_STICKY
  }
  override fun onBind(intent: Intent?) = null

  private fun connect() {
    if (stopped) return
    handler.removeCallbacks(retry)
    if (checkSelfPermission(Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) {
      LinkState.update("error", "Consenti l’accesso ai dispositivi vicini")
      stopSelf()
      return
    }
    val adapter = getSystemService(BluetoothManager::class.java).adapter
    if (adapter?.isEnabled != true) {
      LinkState.update("error", "Bluetooth spento: riattivalo sul telefono")
      return
    }
    val address = getSharedPreferences("bianca_link", MODE_PRIVATE).getString("address", null)
    if (address == null) { stopSelf(); return }
    handler.removeCallbacks(timeout)
    gatt?.close()
    LinkState.update("connecting", "Collegamento a Bianca…")
    gatt = adapter.getRemoteDevice(address.uppercase(Locale.ROOT)).connectGatt(this, false, callback, BluetoothDevice.TRANSPORT_LE)
    handler.postDelayed(timeout, 90000)
  }

  private fun dropCommands(message: String) {
    handler.removeCallbacks(commandTimeout)
    commands.clear()
    val pending = listOfNotNull(batch) + batches
    batch = null
    batches.clear()
    notificationInFlight = null
    pending.forEach { it.done(message) }
  }

  private fun failed(message: String) {
    dropCommands(message)
    handler.removeCallbacks(timeout)
    gatt?.close()
    gatt = null
    if (stopped) return
    LinkState.update("error", message)
    handler.removeCallbacks(retry)
    val delays = longArrayOf(1000, 2000, 5000, 10000, 30000)
    handler.postDelayed(retry, delays[attempt.coerceAtMost(delays.lastIndex)])
    attempt++
  }

  private fun onGatt(current: BluetoothGatt, block: () -> Unit) {
    handler.post {
      if (!stopped && gatt === current) {
        try { block() } catch (_: SecurityException) {
          LinkState.update("error", "Permesso Bluetooth revocato")
          stopSelf()
        }
      }
    }
  }

  private val callback = object : BluetoothGattCallback() {
    override fun onConnectionStateChange(current: BluetoothGatt, status: Int, newState: Int) = onGatt(current) {
      if (status == BluetoothGatt.GATT_SUCCESS && newState == BluetoothProfile.STATE_CONNECTED) {
        if (!current.requestMtu(185)) failed("Impossibile configurare il trasferimento Bluetooth")
      } else if (newState == BluetoothProfile.STATE_DISCONNECTED || status != BluetoothGatt.GATT_SUCCESS) {
        failed("Molletta non raggiungibile, riprovo…")
      }
    }
    override fun onMtuChanged(current: BluetoothGatt, mtu: Int, status: Int) = onGatt(current) {
      if (status != BluetoothGatt.GATT_SUCCESS || mtu < 55) failed("MTU Bluetooth insufficiente")
      else if (!current.discoverServices()) failed("Impossibile leggere i servizi Bluetooth")
    }
    override fun onServicesDiscovered(current: BluetoothGatt, status: Int) = onGatt(current) {
      val characteristic = current.getService(Protocol.service)?.getCharacteristic(Protocol.time)
      if (status != BluetoothGatt.GATT_SUCCESS || characteristic == null) {
        failed("Il dispositivo non espone il protocollo Bianca")
      } else {
        LinkState.update("pairing", "Conferma l’abbinamento con il codice sul cartello di Bianca")
        // La scrittura protetta avvia il bonding di sistema; Android riprende l’operazione dopo il PIN.
        if (current.writeCharacteristic(characteristic, Protocol.timePacket(), BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT) != BluetoothStatusCodes.SUCCESS) {
          failed("Impossibile sincronizzare l’ora")
        }
      }
    }
    override fun onCharacteristicWrite(current: BluetoothGatt, characteristic: BluetoothGattCharacteristic, status: Int) = onGatt(current) {
      if (characteristic.uuid == Protocol.notify) {
        if (status == BluetoothGatt.GATT_SUCCESS) writeNext() else failed("Comando rifiutato dalla molletta")
      } else if (status != BluetoothGatt.GATT_SUCCESS) {
        failed("Abbinamento o sincronizzazione dell’ora non riusciti")
      } else if (characteristic.uuid == Protocol.time) {
        val state = current.getService(Protocol.service)?.getCharacteristic(Protocol.status)
        val descriptor = state?.getDescriptor(Protocol.cccd)
        if (state == null || descriptor == null || !current.setCharacteristicNotification(state, true) ||
          current.writeDescriptor(descriptor, BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE) != BluetoothStatusCodes.SUCCESS) {
          failed("Impossibile attivare gli aggiornamenti della molletta")
        }
      }
    }
    override fun onDescriptorWrite(current: BluetoothGatt, descriptor: BluetoothGattDescriptor, status: Int) = onGatt(current) {
      if (status != BluetoothGatt.GATT_SUCCESS || !current.readCharacteristic(descriptor.characteristic)) {
        failed("Impossibile leggere lo stato della molletta")
      }
    }
    override fun onCharacteristicRead(current: BluetoothGatt, characteristic: BluetoothGattCharacteristic, value: ByteArray, status: Int) = onGatt(current) {
      if (status == BluetoothGatt.GATT_SUCCESS) received(value) else failed("Lettura dello stato non riuscita")
    }
    override fun onCharacteristicChanged(current: BluetoothGatt, characteristic: BluetoothGattCharacteristic, value: ByteArray) = onGatt(current) {
      received(value)
    }
  }

  private fun received(value: ByteArray) {
    val status = Protocol.decodeStatus(value)
    if (status == null) { failed("Versione del protocollo non supportata"); return }
    handler.removeCallbacks(timeout)
    attempt = 0
    val justConnected = LinkState.value["phase"] != "connected"
    LinkState.update("connected", "Bianca è collegata", status)
    if (justConnected) applyActiveBundle()
    NotificationRelay.flush(this)
  }

  override fun onDestroy() {
    stopped = true
    instance = null
    dropCommands("Molletta scollegata")
    handler.removeCallbacksAndMessages(null)
    unregisterReceiver(bluetoothReceiver)
    gatt?.close()
    gatt = null
    LinkState.update("disconnected", "Molletta scollegata")
    super.onDestroy()
  }
}
