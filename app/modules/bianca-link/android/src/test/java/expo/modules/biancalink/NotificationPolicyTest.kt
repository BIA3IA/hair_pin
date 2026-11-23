package expo.modules.biancalink

import org.junit.Assert.*
import org.junit.Test

class NotificationPolicyTest {
  private val item = NotificationItem("key", "pkg", "App", "Giulia", "Caffè 💜", 1000)
  @Test fun disabledOngoingSummaryAndDuplicatesAreRejected() {
    assertFalse(NotificationPolicy.eligible(false, false, false, null, 0))
    assertFalse(NotificationPolicy.eligible(true, true, false, null, 0))
    assertFalse(NotificationPolicy.eligible(true, false, true, null, 0))
    assertFalse(NotificationPolicy.eligible(true, false, false, 1000, 30999))
    assertTrue(NotificationPolicy.eligible(true, false, false, 1000, 31000))
  }
  @Test fun contentDependsOnMode() {
    fun text(mode: String) = NotificationPolicy.packet(item, mode).drop(4).toByteArray().toString(Charsets.UTF_8)
    assertEquals("App", text("app")); assertEquals("Giulia", text("sender"))
    assertEquals("Giulia: Caffè 💜", text("preview"))
  }
  @Test fun truncationNeverSplitsUtf8() {
    assertEquals("è".repeat(24), NotificationPolicy.truncate("è".repeat(25)))
    assertEquals("a".repeat(47), NotificationPolicy.truncate("a".repeat(47) + "💜"))
    assertEquals("a b", NotificationPolicy.truncate("a\nb"))
    val packet = NotificationPolicy.packet(item.copy(body="è".repeat(100)), "preview")
    assertTrue(packet.size <= 52); assertEquals(0, packet[2].toInt()); assertEquals(packet.size - 4, packet[3].toInt())
  }
  @Test fun groupsNeverExposeSenderOrBodyInAnyMode() {
    val group = item.copy(title="Giulia", body="contenuto privato", isGroup=true, conversationTitle="Amici")
    for (mode in listOf("app", "sender", "preview")) {
      val text = NotificationPolicy.packet(group, mode).drop(4).toByteArray().toString(Charsets.UTF_8)
      assertEquals("Amici", text)
    }
    val unknown = NotificationPolicy.packet(group.copy(conversationTitle=""), "preview")
    assertEquals("Gruppo", unknown.drop(4).toByteArray().toString(Charsets.UTF_8))
  }
  @Test fun appIconsMatchWhatsAppAndTelegram() {
    assertEquals(9, NotificationPolicy.packet(item.copy(pkg="com.whatsapp"), "sender")[1].toInt())
    assertEquals(9, NotificationPolicy.packet(item.copy(pkg="com.whatsapp.w4b"), "sender")[1].toInt())
    assertEquals(10, NotificationPolicy.packet(item.copy(pkg="org.telegram.messenger"), "sender")[1].toInt())
    assertEquals(5, NotificationPolicy.packet(item, "sender")[1].toInt())
  }
  @Test fun removesMessageCountsWithoutRemovingRealNumbersInNames() {
    assertEquals("Amici", NotificationPolicy.cleanTitle("Amici (5 messaggi)"))
    assertEquals("Amici", NotificationPolicy.cleanTitle("Amici (5)", 5))
    assertEquals("Amici (5)", NotificationPolicy.cleanTitle("Amici (5)"))
    assertEquals("", NotificationPolicy.cleanTitle("5 messaggi"))
    assertEquals("Giulia", NotificationPolicy.cleanTitle("Giulia [2 messages]"))
    assertEquals("Gruppo 2026", NotificationPolicy.cleanTitle("Gruppo 2026"))
    assertEquals("Giulia", NotificationPolicy.cleanTitle("3 nuovi messaggi: Giulia"))
  }
  @Test fun instagramContainsOnlyItsIconAndOtherAppsAreUnsupported() {
    val packet = NotificationPolicy.packet(item.copy(pkg="com.instagram.android", isGroup=true, conversationTitle="Segreto"), "preview")
    assertEquals(4, packet.size); assertEquals(11, packet[1].toInt()); assertEquals(0, packet[3].toInt())
    assertFalse(NotificationPolicy.supported("com.other.app"))
    assertTrue(NotificationPolicy.supported("org.telegram.messenger"))
  }
  @Test fun pendingExpiresAfterTenMinutes() {
    assertTrue(NotificationPolicy.fresh(item, 601000)); assertFalse(NotificationPolicy.fresh(item, 601001))
  }
}
