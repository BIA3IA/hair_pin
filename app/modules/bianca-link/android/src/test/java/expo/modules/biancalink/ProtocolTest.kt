package expo.modules.biancalink

import org.junit.Assert.*
import org.junit.Test
import java.util.Calendar

class ProtocolTest {
  @Test fun commandsPreserveUtf8AndEnforceLimits() {
    val packet = Protocol.command(2, "Caffè")
    assertEquals(2, packet[2].toInt())
    assertEquals(6, packet[3].toInt())
    assertEquals("Caffè", packet.copyOfRange(4, packet.size).toString(Charsets.UTF_8))
    assertEquals(4, Protocol.command(3, "").size)
    assertThrows(IllegalArgumentException::class.java) { Protocol.command(2, "è".repeat(25)) }
    assertThrows(IllegalArgumentException::class.java) { Protocol.command(4, "") }
    assertThrows(IllegalArgumentException::class.java) { Protocol.command(8, "") }
  }
  @Test fun brightnessAndSleepCommands() {
    val packet = Protocol.command(Protocol.BRIGHTNESS, "128")
    assertEquals(6, packet[2].toInt())
    assertEquals("128", packet.copyOfRange(4, packet.size).toString(Charsets.UTF_8))
    assertArrayEquals(byteArrayOf(1, 0, 7, 0), Protocol.command(Protocol.SLEEP, ""))
    assertThrows(IllegalArgumentException::class.java) { Protocol.command(Protocol.BRIGHTNESS, "5") }
    assertThrows(IllegalArgumentException::class.java) { Protocol.command(Protocol.BRIGHTNESS, "256") }
    assertThrows(IllegalArgumentException::class.java) { Protocol.command(Protocol.BRIGHTNESS, "alta") }
    assertThrows(IllegalArgumentException::class.java) { Protocol.command(Protocol.SLEEP, "x") }
  }
  @Test fun statusDecodesSleepingBatteryAndBrightness() {
    val result = Protocol.decodeStatus(byteArrayOf(3, 1, -1, -128))!!
    assertEquals(true, result["sleeping"])
    assertEquals(255, result["battery"])
    assertEquals(128, result["brightness"])
    assertEquals(false, Protocol.decodeStatus(byteArrayOf(3, 0, 80, 10))!!["sleeping"])
  }
  @Test fun statusRejectsWrongVersionAndLength() {
    assertNull(Protocol.decodeStatus(byteArrayOf()))
    assertNull(Protocol.decodeStatus(byteArrayOf(1, 0, 0)))
    assertNull(Protocol.decodeStatus(byteArrayOf(2, 0, 80))) // versione precedente senza luminosità
    assertNull(Protocol.decodeStatus(byteArrayOf(1, 3, -1, -1, -1, -1, 1, -1))) // vecchio formato con umore e passi
  }
  @Test fun timeUsesFirmwareSevenByteLocalFormat() {
    val before = Calendar.getInstance()
    val packet = Protocol.timePacket()
    val after = Calendar.getInstance()
    assertEquals(7, packet.size)
    val year = (packet[0].toInt() and 255) or ((packet[1].toInt() and 255) shl 8)
    assertTrue(year in before.get(Calendar.YEAR)..after.get(Calendar.YEAR))
    assertTrue(packet[2].toInt() in 1..12)
    assertTrue(packet[3].toInt() in 1..31)
    assertTrue(packet[4].toInt() in 0..23)
    assertTrue(packet[5].toInt() in 0..59)
    assertTrue(packet[6].toInt() in 0..59)
  }
}
