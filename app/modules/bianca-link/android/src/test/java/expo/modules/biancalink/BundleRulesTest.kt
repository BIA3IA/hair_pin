package expo.modules.biancalink

import org.junit.Assert.assertArrayEquals
import org.junit.Assert.assertEquals
import org.junit.Assert.assertThrows
import org.junit.Test

class BundleRulesTest {
  @Test fun cleanDropsBlankLinesAndTrims() {
    assertEquals(listOf("ciao", "a dopo"), BundleRules.clean(listOf("  ciao ", "", "   ", "a dopo")))
  }

  @Test fun acceptsUpToSixtyFourPhrases() {
    BundleRules.validate("Tante", List(64) { "frase $it" })
    assertThrows(IllegalArgumentException::class.java) { BundleRules.validate("Troppe", List(65) { "f" }) }
  }

  @Test fun rejectsEmptyNameAndLongPhrases() {
    assertThrows(IllegalArgumentException::class.java) { BundleRules.validate("  ", listOf("ok")) }
    assertThrows(IllegalArgumentException::class.java) { BundleRules.validate("Nome", listOf("è".repeat(25))) }
    BundleRules.validate("Nome", listOf("è".repeat(24))) // 48 byte esatti
  }

  @Test fun bundleIsFramedByStartAndCommit() {
    val packets = Protocol.bundle(listOf("uno", "due"))
    assertEquals(listOf(3, 4, 4, 5), packets.map { it[2].toInt() })
    assertArrayEquals(byteArrayOf(1, 0, 4, 3) + "uno".toByteArray(), packets[1])
  }

  @Test fun emptyBundleRestoresClipPhrases() {
    assertEquals(listOf(3, 5), Protocol.bundle(null).map { it[2].toInt() })
  }
}
