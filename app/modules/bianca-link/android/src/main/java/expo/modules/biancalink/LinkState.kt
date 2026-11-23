package expo.modules.biancalink

internal object LinkState {
  @Volatile var value: Map<String, Any?> = mapOf("phase" to "disconnected", "message" to "Molletta non collegata", "status" to null)
    private set
  val observers = java.util.concurrent.CopyOnWriteArraySet<(Map<String, Any?>) -> Unit>()
  fun update(phase: String, message: String, status: Map<String, Any>? = null) {
    value = mapOf("phase" to phase, "message" to message, "status" to status)
    observers.toList().forEach { it(value) }
  }
}
