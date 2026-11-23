package expo.modules.biancalink

import android.content.Context
import org.json.JSONArray
import org.json.JSONObject
import java.util.UUID

internal data class PhraseBundle(val id: String, val name: String, val phrases: List<String>)

// Regole condivise con il firmware: al massimo 64 frasi da 48 byte UTF-8.
internal object BundleRules {
  const val MAX_PHRASES = 64
  const val MAX_BYTES = 48
  const val MAX_NAME = 30

  fun clean(phrases: List<String>) = phrases.map { it.trim() }.filter { it.isNotEmpty() }

  fun validate(name: String, phrases: List<String>) {
    require(name.isNotBlank() && name.trim().length <= MAX_NAME) { "Dai un nome al bundle (massimo $MAX_NAME caratteri)" }
    require(phrases.size in 1..MAX_PHRASES) { "Un bundle contiene da 1 a $MAX_PHRASES frasi" }
    require(phrases.all { it.toByteArray(Charsets.UTF_8).size <= MAX_BYTES }) { "Ogni frase può occupare al massimo $MAX_BYTES byte" }
  }
}

internal class PhraseBundles(context: Context) {
  private val prefs = context.getSharedPreferences("bianca_bundles", Context.MODE_PRIVATE)

  fun all(): List<PhraseBundle> {
    val list = JSONArray(prefs.getString("bundles", "[]"))
    return (0 until list.length()).map { i ->
      val entry = list.getJSONObject(i)
      val phrases = entry.getJSONArray("phrases")
      PhraseBundle(entry.getString("id"), entry.getString("name"), (0 until phrases.length()).map(phrases::getString))
    }
  }

  // null = le frasi di Bianca salvate sulla molletta
  fun active(): PhraseBundle? = prefs.getString("active", null)?.let { id -> all().find { it.id == id } }

  fun save(id: String?, name: String, phrases: List<String>): String {
    val cleaned = BundleRules.clean(phrases)
    BundleRules.validate(name, cleaned)
    val bundle = PhraseBundle(id ?: UUID.randomUUID().toString(), name.trim(), cleaned)
    val updated = all().filter { it.id != bundle.id } + bundle
    write(updated.sortedBy { it.name.lowercase() })
    return bundle.id
  }

  fun delete(id: String) {
    write(all().filter { it.id != id })
    if (prefs.getString("active", null) == id) setActive(null)
  }

  fun setActive(id: String?) {
    require(id == null || all().any { it.id == id }) { "Bundle non trovato" }
    prefs.edit().putString("active", id).apply()
  }

  fun state(): Map<String, Any?> = mapOf(
    "active" to active()?.id,
    "bundles" to all().map { mapOf("id" to it.id, "name" to it.name, "phrases" to it.phrases) },
  )

  private fun write(bundles: List<PhraseBundle>) {
    val list = JSONArray()
    bundles.forEach { list.put(JSONObject().put("id", it.id).put("name", it.name).put("phrases", JSONArray(it.phrases))) }
    prefs.edit().putString("bundles", list.toString()).apply()
  }
}
