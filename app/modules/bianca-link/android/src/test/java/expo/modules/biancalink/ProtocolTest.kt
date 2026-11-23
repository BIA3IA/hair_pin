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
    assertThrows(IllegalArgumentException::class.java) { Protocol.command(6, "x") }
  }
  @Test fun statusDecodesUnsignedStepsAndFlags() {
    val result = Protocol.decodeStatus(byteArrayOf(1, 3, -1, -1, -1, -1, 1, -1))!!
    assertEquals(4294967295L, result["steps"])
    assertEquals(3, result["mood"])
    assertEquals(true, result["sleeping"])
    assertEquals(255, result["battery"])
  }
  @Test fun statusRejectsWrongVersionAndLength() {
    assertNull(Protocol.decodeStatus(byteArrayOf()))
    assertNull(Protocol.decodeStatus(byteArrayOf(2, 0, 0, 0, 0, 0, 0, 0)))
    assertNull(Protocol.decodeStatus(ByteArray(9)))
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
