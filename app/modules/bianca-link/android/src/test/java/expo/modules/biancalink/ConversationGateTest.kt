package expo.modules.biancalink
import org.junit.Assert.*
import org.junit.Test

class ConversationGateTest {
  @Test fun sixthMessageMutesOnlyThatChatUntilItsNotificationsAreRemoved() {
    val gate = ConversationGate()
    for (i in 1..5) assertTrue(gate.accept("group", "key", "$i", i))
    assertFalse(gate.accept("group", "key", "6", 6))
    assertFalse(gate.accept("group", "key", "7", 7))
    assertTrue(gate.accept("person", "other", "1", 1))
    gate.remove("key")
    assertTrue(gate.accept("group", "key", "8", 1))
  }
  @Test fun repeatedCallbackDoesNotCountAndAllConversationKeysMustBeRemoved() {
    val gate = ConversationGate()
    assertTrue(gate.accept("chat", "a", "same", 1))
    repeat(10) { assertFalse(gate.accept("chat", "a", "same", 1)) }
    assertFalse(gate.muted("chat"))
    assertFalse(gate.accept("chat", "b", "new", 6))
    gate.remove("a")
    assertFalse(gate.accept("chat", "b", "next", 1))
    gate.remove("b")
    assertTrue(gate.accept("chat", "c", "first", 1))
  }
}
