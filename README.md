# Linux kernel per Doogee S88 Pro (`s88pro`)

Kernel Linux 4.14.141 per il Doogee S88 Pro, basato sull'albero ALPS di
MediaTek per MT6771 ([mtk-watch/android_kernel-4.14](https://github.com/mtk-watch/android_kernel-4.14),
commit `952d88e94`).

## Perché questo repository esiste

Doogee non ha mai pubblicato i sorgenti del kernel di questo dispositivo.
Finché non c'erano, il device tree LineageOS era costretto a spedire un
kernel precompilato estratto dal `boot.img` originale — cosa che i requisiti
di supporto di LineageOS vietano esplicitamente:

> Non-GKI devices MUST NOT ship a prebuilt kernel.

Le correzioni in questo albero sono state **ricostruite leggendo il kernel di
fabbrica** — disassemblato e mappa dei simboli — e verificate sul dispositivo.
Ogni commit porta nel messaggio le prove su cui si regge: indirizzi,
istruzioni citate per opcode, e la misura che ha confermato la correzione.

## Compilazione

Il kernel si compila attraverso il sistema di build di LineageOS. Il device
tree lo dichiara così:

```make
TARGET_KERNEL_SOURCE := kernel/doogee/s88pro
TARGET_KERNEL_CONFIG := lineage_s88pro_defconfig
BOARD_KERNEL_IMAGE_NAME := Image.gz-dtb
```

Per compilarlo da solo:

```bash
make O=out ARCH=arm64 CC=clang CLANG_TRIPLE=aarch64-linux-gnu- \
     CROSS_COMPILE=aarch64-linux-android- lineage_s88pro_defconfig
make O=out ARCH=arm64 -j$(nproc) CC=clang CLANG_TRIPLE=aarch64-linux-gnu- \
     CROSS_COMPILE=aarch64-linux-android- Image.gz-dtb
```

**Nota sul compilatore**: l'albero è stato ricostruito e verificato con
`clang-r353983c`, la stessa versione usata dal kernel di fabbrica (che si
dichiara come `clang version 9.0.3 ... r353983c`). Con compilatori più
recenti il kernel può compilare ugualmente, ma il confronto istruzione per
istruzione con il binario originale — il metodo con cui questo albero è stato
verificato — vale solo con quella versione.

## Che cosa è stato corretto, e come si verifica

Le correzioni non sono congetture: ognuna nasce da una misura sul binario di
fabbrica e finisce con una misura sul dispositivo. Alcune fra le principali:

| area | difetto | misura di conferma |
|---|---|---|
| camere | il modulo IMX230 veniva scelto senza leggere l'EEPROM che distingue i due fornitori: immagine ruotata e Bayer sbagliato | `/proc/driver/camera_info` identico riga per riga a quello di fabbrica |
| audio | l'amplificatore AW87329 non veniva acceso da nessuno: `aw87329_audio_kspk` era compilata e irraggiungibile | `hwen` passa da 0 a 1 da solo quando Android riproduce |
| jack cuffie | `CONFIG_ACCDET_EINT` acceso faceva fallire il probe con `-ENODEV` | `/dev/accdet` con lo stesso major/minor dello stock |
| TEE | il clock SPI non era registrato: 64 errori per avvio e Trusted Application non caricate | errori da 64 a 0, `tee_ta_load` come nello stock |
| avvio | `REVERSE_CHARGER` mancante dall'enum mandava il telefono in bootloop | il telefono si avvia |

## Stato

Verificato sul dispositivo: camere (entrambe), audio, jack, sensori (luce,
prossimità, accelerometro, magnetometro), touch, LED e torcia, NFC, lettore di
impronte, carica senza fili, TEE.

Uno stress test appaiato contro il kernel di fabbrica — sospensione e
risveglio, camere, audio, grafica, storage, carico CPU, rete, tre giri per
parte — non ha prodotto panici, BUG, call trace o WARNING da nessuna delle due
parti, e gli altri messaggi compaiono negli stessi ordini di grandezza.

## Licenza

GPL v2, come il kernel Linux. Vedi [COPYING](COPYING).
