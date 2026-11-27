package expo.modules.biancalink

import java.nio.ByteBuffer
import java.nio.ByteOrder
import java.util.Calendar
import java.util.UUID

internal object Protocol {
  val service = UUID.fromString("1c0017b5-a1e0-46a8-b545-40745eeb96d1")
  val notify = UUID.fromString("cdcb32b1-2a88-4051-b7d3-adbc1f51e60e")
  val time = UUID.fromString("f4b0c127-e20d-433d-886e-d2494f3a8535")
  val status = UUID.fromString("6b4747a6-1e7f-48c9-aa59-ddd21c5a844d")
  val cccd = UUID.fromString("00002902-0000-1000-8000-00805f9b34fb")

  const val BRIGHTNESS = 6
  const val SLEEP = 7
  const val MIN_BRIGHTNESS = 10

  // azioni: 1 animazione, 2 cartello, 3-4-5 bundle (inizio, frase, conferma), 6 luminosità, 7 dormi
  fun command(action: Int, text: String): ByteArray {
    require(action in 1..SLEEP) { "Comando non valido" }
    require(action != BRIGHTNESS || text.toIntOrNull() in MIN_BRIGHTNESS..255) { "Luminosità non valida" }
    val bytes = text.toByteArray(Charsets.UTF_8)
    require(bytes.size <= 48) { "La frase supera 48 byte UTF-8" }
    require(action !in listOf(1, 2, 4) || text.isNotBlank()) { "Testo vuoto" }
    require(action !in listOf(3, 5, SLEEP) || text.isEmpty()) { "Comando senza testo richiesto" }
    return byteArrayOf(1, 0, action.toByte(), bytes.size.toByte()) + bytes
  }

  // senza frasi la molletta torna alle sue
  fun bundle(phrases: List<String>?): List<ByteArray> =
    listOf(command(3, "")) + phrases.orEmpty().map { command(4, it) } + command(5, "")

  fun timePacket(): ByteArray {
    val now = Calendar.getInstance()
    return ByteBuffer.allocate(7).order(ByteOrder.LITTLE_ENDIAN)
      .putShort(now.get(Calendar.YEAR).toShort())
      .put((now.get(Calendar.MONTH) + 1).toByte())
      .put(now.get(Calendar.DAY_OF_MONTH).toByte())
      .put(now.get(Calendar.HOUR_OF_DAY).toByte())
      .put(now.get(Calendar.MINUTE).toByte())
      .put(now.get(Calendar.SECOND).toByte()).array()
  }

  // STATUS v3: versione, flag (bit 0 = dorme), batteria (255 = sconosciuta), luminosità
  fun decodeStatus(value: ByteArray): Map<String, Any>? {
    if (value.size != 4 || value[0].toInt() != 3) return null
    return mapOf(
      "sleeping" to (value[1].toInt() and 1 != 0),
      "battery" to (value[2].toInt() and 255),
      "brightness" to (value[3].toInt() and 255))
  }
}
