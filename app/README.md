# Bianca per Android

Expo SDK 57, TypeScript, NativeWind 5 RC; pnpm. La build di sviluppo richiede JDK 17 e Android SDK 36. Il collegamento Bluetooth richiede Android 13 o successivo.

## Sviluppo

- `pnpm install --frozen-lockfile`
- `pnpm android` per compilare e installare sul telefono USB.
- `pnpm start` per Metro; `adb reverse tcp:8081 tcp:8081` per usare la connessione USB.
- `pnpm typecheck` e `pnpm lint` per verificare il codice.
- `JAVA_HOME=/usr/lib/jvm/java-17-openjdk ANDROID_HOME=$HOME/Android/Sdk ./android/gradlew -p android :bianca-link:testDebugUnitTest` per i test Kotlin del protocollo.

## Collegamento

Il modulo locale `modules/bianca-link` usa CompanionDeviceManager per scegliere Bianca. LinkService possiede la connessione GATT e resta in foreground con una notifica di sistema; sincronizza l’ora, sottoscrive STATUS e tenta la riconnessione con attese crescenti fino a 30 secondi. Il manifest della libreria dichiara permessi e servizio e viene unito al manifest dell’app da Gradle.

Accendere la molletta, attivare Bluetooth e posizione sul telefono, toccare “Collega la molletta” e consentire i dispositivi vicini. Scegliere Bianca nella finestra Android e inserire il codice mostrato sul cartello della molletta. Il firmware accetta nuovi abbinamenti nei primi due minuti dall’accensione. L’associazione Android viene conservata; “Scollega” ferma il servizio, “Rimuovi associazione” permette di ricominciare dopo un errore. La rimozione non cancella il bond Bluetooth del sistema.

L’inoltro delle notifiche di altre app non è ancora implementato. Restano da verificare sul dispositivo il primo abbinamento, la lettura dello stato e la stabilità di un’ora in background. La build debug dipende da Metro per caricare JavaScript all’avvio.

## Controlli e anteprima cartello

L’icona Bluetooth apre il popup di collegamento. Le animazioni manuali partono una volta. I bundle (massimo 8 frasi) sostituiscono le frasi spontanee fino alla disconnessione BLE; poi tornano quelle predefinite. Il firmware deve essere aggiornato insieme al modulo Android per i nuovi comandi.

L’anteprima usa i bitmap U8g2 estratti con `python3 ../tools/export_sign_fonts.py`, dopo aver installato le librerie firmware con PlatformIO. `python3 ../tools/sign_preview/reference.py` rigenera i raster di riferimento usando le funzioni wrap/draw_text del C++ firmware con un display host. `node --test test/*.test.mjs` verifica soglie umore e raster di frasi, accenti e ritorni a capo. Le frasi che non entrano o usano caratteri assenti nei font sono segnalate prima dell’invio.

## Notifiche del telefono

Aprire “Notifiche” → “Apri accesso notifiche” e concedere a Bianca l’accesso nella schermata Android. Attendere una nuova notifica dall’app desiderata, aggiornare l’elenco e abilitarla scegliendo nome app, mittente o mittente + anteprima. La prima notifica serve a popolare l’elenco; non viene inoltrata perché l’app nuova è disabilitata. Nessun silenzio, nemmeno notturno. Il firmware deve includere la rimozione dello scarto notturno.

Test: `./android/gradlew -p android :bianca-link:testDebugUnitTest` con JDK 17 e ANDROID_HOME; test JS con `node --test test/*.test.mjs`. Per provare senza messaggi reali si può pubblicare una notifica locale sintetica dal PC con `adb shell cmd notification post -t 'Test Bianca' bianca-test 'Messaggio di prova'`, abilitare l’app di origine nella schermata Notifiche, quindi ripetere con una chiave diversa. Non concedere accessi di sistema tramite adb al posto dell’utente.
