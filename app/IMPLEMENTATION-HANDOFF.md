# Collegamento Android alla molletta

Implementato il modulo locale Kotlin `modules/bianca-link`: selezione con CompanionDeviceManager, persistenza dell’associazione, connessione GATT in foreground service, sincronizzazione TIME, lettura e sottoscrizione STATUS, timeout e riconnessione con backoff. La home usa `useBianca` per mostrare stato, umore e passi reali quando connessa, con azioni per scollegare e rimuovere l’associazione dopo un errore.

## Verifica

- Typecheck e lint: passati.
- Build Android debug e installazione sul Flip: riuscite.
- Test JVM ProtocolTest: tre test passati (STATUS senza segno, rifiuto di formato/versione errati, formato TIME).
- Primo abbinamento con PIN, dati effettivi, riavvio della molletta e stabilità per un’ora in background: da verificare con l’utente.
- Inoltro delle notifiche e avvio automatico dopo riavvio del telefono: milestone successivi.
- Compatibilità del collegamento: Android 13+; build debug con Metro.

## Revisione

Letti integralmente i file del modulo (package.json, expo-module.config.json, build.gradle, AndroidManifest.xml, index.ts, BiancaLinkModule.kt, LinkService.kt, LinkState.kt, Protocol.kt e ProtocolTest.kt), useBianca.ts e la home modificata. Revisione secondo find-bugs: ingressi GATT validati per versione e lunghezza, permessi richiesti prima dell’avvio, servizio non esportato, nessun contenuto di notifiche o PIN nei log. Verificati annullamento, callback di connessioni precedenti, risorse e ritenti limitati; corretto un possibile ritento sovrapposto. SQL, XSS, CSRF, sessioni web e crittografia applicativa non applicabili. Bonding e cifratura dipendono dallo stack Android e dal firmware esistente; comportamento sul dispositivo ancora da provare.

## Testi suggeriti

Branch: `feat/android-clip-link`

Conventional Commit: `feat(app): connect Android companion to Bianca over BLE`

Titolo PR: `Collega l’app Android alla molletta Bianca`

Descrizione PR:

L’app può scegliere Bianca tramite la finestra di associazione Android e avviare una connessione BLE gestita da un servizio in foreground. Sincronizza l’ora, legge umore e passi, si sottoscrive agli aggiornamenti e tenta la riconnessione quando la molletta non è raggiungibile. La home mostra gli stati di collegamento e consente di scollegare o rimuovere l’associazione.

Verifica: typecheck, lint, tre test JVM del protocollo e build/installazione debug sul Flip riusciti. Restano la prova di abbinamento con PIN e la verifica di stabilità in background. Inoltro delle notifiche escluso da questa modifica.

Nessun commit, push, PR, merge o deploy eseguito. Modifiche lasciate nel working tree; conservato il lavoro preesistente.

## Correzione crash dopo associazione

Log del Flip: `IllegalArgumentException: fc:01:2c:fe:08:36 is not a valid Bluetooth address` in `LinkService.connect`. CompanionDeviceManager restituisce il MAC in minuscolo; BluetoothAdapter richiede le lettere in maiuscolo. Corretto il salvataggio dell’indirizzo in BiancaLinkModule e la lettura in LinkService con `uppercase(Locale.ROOT)`, compatibile anche con le associazioni già salvate.

Regressione manuale: conservare l’associazione con indirizzo minuscolo già salvata, aprire l’app, premere Collega, verificare che LinkService arrivi alla connessione GATT senza IllegalArgumentException. Completare il PIN sul telefono per verificare il bonding (fuori dal punto del crash). Non serve cancellare dati o associazione.

Commit suggerito per la correzione: `fix(app): normalize Bluetooth addresses before connecting`

Titolo PR: `Correggi il crash Bluetooth dopo l’associazione`

Descrizione PR: Normalizza gli indirizzi MAC restituiti da CompanionDeviceManager prima del salvataggio e prima della connessione GATT. Gestisce anche gli indirizzi minuscoli già persistiti, evitando IllegalArgumentException all’avvio del servizio senza richiedere un nuovo abbinamento.

Verifica della correzione sul Flip: riutilizzata l’associazione persistita in minuscolo, l’app ha superato `getRemoteDevice` ed è arrivata alla finestra Android “Associare con Bianca?” senza chiudersi. Build aggiornata/installazione, typecheck, lint e tre test Kotlin passati. La conferma di sistema e il successivo PIN restano all’utente; connessione completa e stabilità non ancora certificate.

## Controlli manuali e anteprime

Sostituito il grande riquadro di collegamento con icona Bluetooth e popup. Corretto l’umore: STATUS contiene 0–100, non un indice 0–3; la UI usa le soglie firmware 25/45/75. Esportate 34 animazioni Idle (430 frame). Aggiunti selettore con anteprima, invio animazione una volta, cartello manuale e bundle temporanei fino a 8 frasi, con anteprima di ciascuna. Il bundle viene cancellato alla disconnessione. Nessuna modifica alle soglie IMU né all’umore salvato: falling_tired è favorita dai pesi GLOOMY quando l’umore è basso.

Protocollo: flags NOTIFY 1 animazione, 2 cartello manuale, 3 inizio staging bundle, 4 aggiunta frase, 5 commit. Liste di comandi serializzate con risposta GATT e timeout, MTU negoziato prima dei servizi; coda firmware di 12 elementi per un massimo di 10 pacchetti per bundle. Whitelist di animazioni e validazione dei pacchetti. Nome animazione mai usato come percorso arbitrario. Bundle in RAM e commit separato per evitare applicazione parziale. La conferma UI indica invio GATT, non la fine della riproduzione.

L’anteprima indicativa iniziale è stata sostituita da bitmap U8g2 estratti dalle librerie firmware e layout equivalente a wrap/draw_text. Il generatore di riferimento esegue le funzioni C++ originali con un display host; 10 raster di esempio sono confrontati pixel per pixel. Il simulatore host usa i dati dei glifi esportati, non una cattura del display fisico: verifica indipendente dell’impaginazione, non del decoder font o del controller LCD. Colore, grassetto, font e geometria del cartello riprodotti nell’app. Test separati per umore, overflow e caratteri non supportati.

Corretto anche l’esportatore sprite: aggiorna i file e l’indice con sostituzioni atomiche senza cancellare la cartella mentre Metro la legge. Il crash osservato durante la generazione derivava da immagini momentaneamente assenti nel catalogo del bundler; dopo esportazione completa e riavvio Metro il bundle risponde HTTP 200 e non risultano nuovi crash.

Verifica: 10 test firmware, 4 test JVM, 14 test JS; typecheck, lint, build firmware e Android debug passati. App installata sul Flip e firmware caricato sulla molletta; nuova UI e anteprime aperte sul Flip. Restano da confermare sul dispositivo invio di ogni comando, bundle/disconnessione, confronto visivo fisico del cartello e stabilità prolungata. Non si certifica ancora un uso giornaliero.

Revisione find-bugs applicata ai nuovi file e alle parti modificate (UI/popup/anteprime, esportatori, trasporto native, protocollo e comportamento firmware, test). Verificati: input e limiti, accesso BLE autenticato ereditato, servizio non esportato, staging e reset sessione, serializzazione GATT, timeout e rilascio risorse, assenza di contenuti/PIN nei log aggiunti. Nessun ingresso SQL/web/sessione applicabile. Nessuna nuova dipendenza di produzione. File preesistenti conservati, nessuna operazione Git di pubblicazione.

Branch suggerito: `feat/clip-controls-preview`
Commit suggerito: `feat(app): add clip controls and firmware-matched sign previews`
Titolo PR: `Aggiungi controlli per animazioni e cartello con anteprima`
Descrizione PR: Il collegamento passa in un popup aperto dall’icona Bluetooth. L’app mostra correttamente il punteggio umore, offre le animazioni disponibili e permette di inviare una riproduzione singola, una scritta o un bundle temporaneo di frasi. L’anteprima del cartello usa i font bitmap e le regole di impaginazione firmware. Comandi GATT serializzati e validati; bundle applicati solo al commit e rimossi alla disconnessione. Verifica automatica: test firmware/JVM/JS, typecheck, lint, build; verifica manuale completa dei comandi e stabilità ancora da terminare.

## Inoltro notifiche Android

Implementati BiancaNotificationListener, NotificationSettings, NotificationPolicy e NotificationRelay. Listener di sistema protetto da BIND_NOTIFICATION_LISTENER_SERVICE; regole per app persistite in SharedPreferences, app nuove disabilitate. Modalità nome app, mittente, mittente + anteprima con avviso nella UI. Nessun contenuto delle notifiche persistito o loggato; ultima notifica in RAM con TTL di dieci minuti, timer unico, rimozione alla cancellazione della notifica, rivalidazione di permessi/regole prima dell’invio. Esclusi notifiche ongoing, riepiloghi e aggiornamenti entro 30 secondi; cache dedup limitata a 256 chiavi.

Il listener condivide la connessione/coda GATT di LinkService con i comandi manuali; una notifica attende la fine dei comandi. Le scritture non si sovrappongono. L’ultima app inviata è metadata solo RAM ed è indicata come invio Bluetooth, non conferma di visualizzazione sul cartello.

Richiesta utente: mai silenzio. Nessun filtro orario introdotto e rimosso lo scarto notturno dal firmware; le notifiche possono svegliare Bianca. Conservati i comportamenti autonomi notturni.

Verifica: 8 test JVM (4 protocollo + 4 filtro/composizione/UTF-8/TTL), 14 test JS, 10 test firmware; typecheck e lint passati. Build debug Android e firmware riuscite, APK installato sul Flip e firmware caricato sulla molletta; bundle Metro HTTP 200. Da completare: concessione utente dell’accesso notifiche e prova end-to-end, revoca del permesso, riconnessione e test prolungato in background. APK autonomo e avvio al boot restano nel milestone di robustezza.

Revisione find-bugs sui quattro nuovi file Kotlin, nuovo pannello UI, API native/TS, manifest, LinkService e variazione firmware. Verificati permessi/binding, input/modalità, truncamento senza spezzare UTF-8, default disabilitato, limiti dedup/TTL, coda condivisa, rivalidazione privacy, mancata persistenza del contenuto. Nessuna superficie web/SQL o nuova dipendenza di produzione. Residuo: integrazione Android reale e stabilità richiedono test sul dispositivo con l’accesso concesso.

Branch suggerito: `feat/android-notification-forwarding`
Conventional Commit: `feat(app): forward selected Android notifications to Bianca`
Titolo PR: `Inoltra le notifiche Android selezionate alla molletta`
Descrizione PR:

Aggiunge un listener Android e impostazioni per scegliere app abilitate e contenuto mostrato sul cartello. Le app nuove partono disabilitate; notifiche ongoing, riepiloghi e duplicati vengono scartati. L’ultima notifica può attendere in RAM dieci minuti e viene rivalidata prima dell’invio sulla connessione GATT condivisa. Nessun silenzio orario, come richiesto: il firmware riceve anche di notte.

Verifica: build Android/firmware, typecheck/lint, 8 test JVM, 14 JS e 10 firmware passati. APK e firmware installati localmente. La prova end-to-end richiede la concessione manuale dell’accesso notifiche Android; stabilità prolungata e release autonoma restano da verificare.

Modifiche lasciate nel working tree; nessun commit, push, PR, merge o deploy eseguito.

## Disposizione notifiche sul cartello

Le icone MESSAGE/PHONE/MAIL/BELL con testo ora sono centrate sopra il testo, che usa tutta la larghezza del cartello nella zona sottostante. Restano 3 pixel di separazione fra le aree; font e a capo vengono scelti in base allo spazio disponibile. I cartelli senza testo e le icone delle frasi spontanee mantengono il comportamento precedente. Modifica limitata a sign_draw, senza variazioni al protocollo o all’app. Verifica: build e caricamento firmware; confronto visivo sulla prossima notifica ancora da confermare.

Commit suggerito: `fix(display): place notification icons above sender text`
Titolo PR: `Centra l’icona sopra il testo delle notifiche`
Descrizione PR: Dispone le icone di notifica sopra mittente e anteprima, centrando entrambe e usando l’intera larghezza per il testo. La selezione automatica dello stile usa l’altezza residua del cartello. Build e caricamento firmware verificati; resa sul display fisico da confermare con una notifica.

## Gruppi e icone WhatsApp/Telegram

Il listener estrae MessagingStyle con NotificationCompat e usa stato di gruppo/titolo della conversazione. Un gruppo mostra solo il titolo del gruppo in tutte le modalità, mai mittente o corpo; se Android segnala un gruppo senza titolo, il testo è “Gruppo”, senza ripiegare sul nome della persona. Le chat private usano il mittente strutturato se disponibile, altrimenti il titolo; restano le modalità app/mittente/anteprima scelte dall’utente. I nomi di conversazioni private non vengono classificati come gruppi quando MessagingStyle segnala esplicitamente una chat privata. Nessuna euristica basata su due punti o sul contenuto del messaggio.

Aggiunte icone native monocromatiche WhatsApp/Business (id 9) e Telegram/Telegram X (id 10). Simboli 20x20, quasi neri sul cartoncino bianco, centrati con margine superiore 4 px, area testo separata da 2 px. Il protocollo accetta ora 11 icone; le precedenti mantengono gli ID. App e firmware aggiornati insieme. I simboli sono rappresentazioni pixel art, non le icone originali raster delle app.

Verifica: build Android, test JVM (inclusi gruppi senza mittente/corpo in ogni modalità e selezione icona per pacchetto), 11 test firmware, typecheck/lint e build/caricamento firmware. APK installato sul Flip. Ancora da confermare con nuove notifiche reali di gruppo e private e foto del cartello: classificazione dipende dai metadati che l’app di origine espone; layout non ancora verificato fisicamente con le nuove icone.

Commit suggerito: `fix(notifications): show group titles and dedicated chat icons`
Titolo PR: `Mostra solo il nome dei gruppi e migliora le icone delle chat`
Descrizione PR: Usa i metadati di conversazione Android per distinguere gruppi e chat private. Nei gruppi esclude sempre persona e messaggio, mantenendo solo il titolo della conversazione. Aggiunge icone monocromatiche WhatsApp e Telegram con margini e spazio testo dedicati sul cartello. Verificati test JVM/firmware, typecheck, lint e build; da confermare il risultato con notifiche reali sul dispositivo. Nessun commit, push o PR eseguito.

## Saluto singolo, sorgenti limitate e blocco dopo cinque messaggi

Il saluto wave era configurato ONCE, che nel player significa avanti e poi indietro: la sequenza sorgente di 17 frame veniva percorsa due volte. Impostato FORWARD per avvio, variante spontanea e comando remoto wave; le altre animazioni mantengono il proprio comportamento. Build e flash firmware passati; ciclo visivo da confermare sul dispositivo.

Filtrate WhatsApp/Business, Telegram/Telegram X e Instagram prima dell’elaborazione. NotificationSettings filtra anche le vecchie regole delle altre app: non compaiono più e non possono inviare. Instagram ha ID icona 11, pacchetto senza testo, logo fotocamera monocromatico centrato; nessuna modalità testo nella UI. Nessuna nuova dipendenza.

ConversationGate conta per ID shortcut o nome conversazione normalizzato, con impronta dell’ultimo messaggio per evitare conteggi duplicati. Dalla sesta notifica/messaggio segnalato, blocca quella chat e cancella l’eventuale pending della stessa conversazione. Il blocco resta finché tutte le sue chiavi di notifica sono rimosse tramite callback Android. Il dedup trasporto di 30 secondi resta separato: non tutte le prime cinque notifiche devono necessariamente apparire. Stato solo RAM, limiti 256 conversazioni/512 chiavi, nuove conversazioni scartate se i limiti sono raggiunti. Non vengono memorizzati corpi di messaggi nel gate né scritti contenuti nei log.

Titoli ripuliti da suffissi espliciti come “(5 messaggi)” e “[2 messages]”; contatori tra parentesi privi di etichetta rimossi solo quando coincidono con notification.number. Numeri reali nei nomi restano. Per il contenuto privato viene preferito l’ultimo messaggio strutturato, evitando riepiloghi di sistema quando disponibile.

Verifica: 14 test JVM (4 protocollo, 8 policy, 2 gate), 11 firmware, typecheck/lint, build Android e firmware riusciti; APK e firmware installati. Restano le prove reali di lettura/cancellazione, sei messaggi nella stessa chat e ciclo del saluto. La lettura viene rilevata tramite rimozione della notifica Android; nessun accesso allo stato di lettura interno di WhatsApp/Telegram. Riavvio del processo azzera lo stato RAM; il conteggio non letto riportato da Android viene usato per ricostruire il blocco quando presente.

Revisione dei file del task: verificati blocco per conversazione, rimozione del pending, nessun invio da sorgenti escluse, logo Instagram sempre privo di testo, limiti di memoria, nessun nuovo dato su disco e preservazione delle modifiche preesistenti. Nessun commit, push, PR, merge o deploy eseguito.

Branch: `fix/notification-bursts-and-wave`
Conventional Commit: `fix: limit notification bursts and play greeting once`
Titolo PR: `Limita le notifiche per chat e riproduci un solo saluto`
Descrizione PR: Corregge il saluto eliminando la riproduzione inversa della sequenza completa. Limita le sorgenti a WhatsApp e Telegram, più Instagram con solo logo, rimuove i conteggi riepilogativi dai titoli e blocca le conversazioni dalla sesta notifica fino alla rimozione delle notifiche della chat sul telefono. Test JVM/firmware, typecheck, lint e build passati; prova completa sul dispositivo ancora da confermare.

## Semantica delle animazioni e nomi più grandi

Corrette le clip complete Caffè, Pensierosa, Posa buffa, Guarda a destra e altre azioni della stessa categoria: avanzano senza ripercorrere i frame al contrario. Equilibrio manuale è un giro in avanti; quello autonomo scorre in avanti senza la pausa/oscillazione HOLD. Perde l’equilibrio usa POSE: raggiunge la posa, resta ferma ed esce senza ripetere l’intera sequenza inversa. Camminata e corsa manuali sono in avanti; il player completa in avanti anche un LOOP interrotto con QUICK.

Le clip manuali mantengono le pose richieste: Cade stanca 20–40 s complessivi prima del ritorno; Si gira 10–15 s. Guarda a destra è singolo da app; nelle variazioni spontanee può ripetersi una seconda volta con probabilità 1/4. La richiesta finale dell’utente identifica Salto felice (little_jump_in_place), non Saltella: usato come idle felice per 25–45 s, con normale selezione pesata dall’umore. Il comando manuale resta un ciclo unico.

Notifiche con testo: icona compatta a 16 px, margine alto 2 px e separazione 2 px, lasciando al testo altezza sufficiente per due righe BIG/MEDIUM quando la larghezza lo consente. Il logo senza testo (Instagram) rimane centrato a 20 px. Nomi molto lunghi continuano a usare uno stile minore secondo lo spazio disponibile.

Aggiunto ambiente PlatformIO native-player con clock e sprite simulati. Cinque test osservano sequenze dei frame, uscita in avanti, posa ferma, attesa prima della risalita e interruzione senza riproduzione inversa. `pio test -e native -e native-player`: 16 test passati (11 protocollo, 5 player). Build firmware e caricamento USB riusciti. Verifica visiva finale sul display ancora da effettuare; nessuna build Android necessaria per queste sole modifiche firmware.

Revisione: modificati player.h/cpp, configurazioni clip in comportamento, geometria notifiche in cartello e ambiente/test player. Conservati HOLD con ritorno per le transizioni che devono rialzarsi o rigirarsi; FORWARD/POSE non eseguono un ritorno inverso se interrotte. Nessuna modifica a protocolli BLE, filtri notifiche, umore persistito o lavoro estraneo.

Branch: `fix/animation-playback-semantics`
Conventional Commit: `fix: correct clip playback and enlarge notification names`
Titolo PR: `Correggi cicli e pose delle animazioni e ingrandisci i nomi`
Descrizione PR: Distingue cicli completi da transizioni di posa, eliminando riproduzioni inverse e oscillazioni inappropriate. Mantiene le pose manuali stanca/di spalle, consente idle felice più lunghi e impedisce passi inversi durante l’uscita da camminata e corsa. Recupera spazio sul cartello per nomi su due righe più grandi. Verificati 16 test nativi, build e caricamento firmware; resa visiva da confermare sulla molletta.

Nessun commit, push, PR o deploy eseguito; working tree lasciato per revisione.

## Nuova grafica, bundle salvati, niente passi

- Home: stelle e cuori pixel attorno a Bianca (stesse forme di `src/display/background.cpp`, `src/bianca/shapes.ts`), icone tonde in alto per notifiche e Bluetooth, pulsante "Sorpresa!" che invia un'animazione a caso, solo l'etichetta dell'umore (niente punteggio). Bordi arrotondati su pulsanti, popup e liste; animazioni in griglia a due colonne di uguale larghezza.
- Bundle di frasi salvati in `PhraseBundles.kt` (SharedPreferences): illimitati, fino a 64 frasi da 48 byte l'uno (limite RAM firmware, doppio buffer con scambio al commit). "Frasi di Bianca" mostra le frasi del firmware generate da `tools/export_default_phrases.ts`. Il bundle scelto viene inviato da `LinkService` a ogni collegamento e quando cambia; scollegata la molletta torna alle sue.
- `LinkService`: coda di gruppi di comandi (notifiche, bundle, comandi manuali) invece del rifiuto "già in corso".
- Passi rimossi da app e cartello su richiesta (anche l'integrazione Health Connect provata e tolta). La molletta conta ancora i passi solo per riconoscere la camminata; STATUS mantiene il campo per compatibilità.
- `minSdkVersion` 33 tramite expo-build-properties (il collegamento richiede comunque Android 13).
- Firmware: "Perde l'equilibrio" torna HOLD (ondeggia e si rialza) invece di restare ferma; rimossa la modalità POSE, sostituita da un test di regressione.

Verifica: 16 test firmware, 19 test JVM, 14 test JS, typecheck, lint, Prettier; build e installazione su Flip e molletta. Home e animazioni controllate da screenshot sul Flip; scheda Bundle, invio automatico alla riconnessione e bundle da 64 frasi da provare a mano.

## Pagine, icona e installazione definitiva

- Animazioni e Cartello sono pagine (`src/app/animazioni.tsx`, `src/app/cartello.tsx`); Bluetooth e Notifiche restano popup, ora centrati (il vecchio `SafeAreaView` con className non applicava né sfondo né centratura). Animazioni raggruppate per umore (`ANIMATION_GROUPS` in `labels.ts`, Record esaustivo); frasi della molletta per categoria (assegnate in `tools/export_default_phrases.ts`, fallback "Altre").
- Notifiche: lista fissa delle app supportate installate (`<queries>` nel manifest del modulo), niente più "Aggiorna elenco".
- Pulsanti: niente spostamento alla pressione; `busy` ignora i tocchi durante l'invio senza cambiare aspetto. `active:` va solo sul Pressable o con `group`/`group-active:` sui figli, altrimenti il figlio cattura il tocco.
- "Si gira" (turn_away) rimossa; al quinto tocco spinta + imbronciata lunga. `export_app_sprites.ts` ora elimina le cartelle delle animazioni rimosse.
- Cartello: notifica con icona grande (20 px loghi app, 24 px icone generiche) subito sopra il nome, blocco centrato; testo diviso in `layout_text`/`render_text`. Rasters di riferimento rigenerati e identici ai precedenti.
- Icona: `tools/make_app_icon.ts` (pixel art con Bianca al centro) genera icon/adaptive/monochrome/splash.
- Installata build release (`./gradlew app:installRelease`, firma debug come la build di sviluppo, quindi dati e abbinamento conservati): funziona senza Metro né USB.
