# Linux kernel for the Doogee S88 Pro (`s88pro`)

Linux kernel 4.14.141 for the Doogee S88 Pro, based on MediaTek's ALPS tree
for the MT6771 ([mtk-watch/android_kernel-4.14](https://github.com/mtk-watch/android_kernel-4.14),
commit `952d88e94`).

## Why this repository exists

Doogee never published the kernel sources of this device. For as long as they
were missing, the LineageOS device tree had to ship a prebuilt kernel
extracted from the original `boot.img` — which the LineageOS support
requirements explicitly forbid:

> Non-GKI devices MUST NOT ship a prebuilt kernel.

The corrections in this tree were **reconstructed by reading the factory
kernel** — its disassembly and its symbol map — and verified on the device.
Every commit carries in its message the evidence it rests on: addresses,
instructions quoted by opcode, and the measurement that confirmed the fix.

## Building

The kernel is built through the LineageOS build system. The device tree
declares it like this:

```make
TARGET_KERNEL_SOURCE := kernel/doogee/s88pro
TARGET_KERNEL_CONFIG := lineage_s88pro_defconfig
BOARD_KERNEL_IMAGE_NAME := Image.gz-dtb
```

To build it on its own:

```bash
make O=out ARCH=arm64 CC=clang CLANG_TRIPLE=aarch64-linux-gnu- \
     CROSS_COMPILE=aarch64-linux-android- lineage_s88pro_defconfig
make O=out ARCH=arm64 -j$(nproc) CC=clang CLANG_TRIPLE=aarch64-linux-gnu- \
     CROSS_COMPILE=aarch64-linux-android- Image.gz-dtb
```

**A note on the compiler**: the tree was reconstructed and verified with
`clang-r353983c`, the same version the factory kernel used (it declares
itself as `clang version 9.0.3 ... r353983c`). With more recent compilers the
kernel may well build all the same, but the instruction-by-instruction
comparison against the original binary — the method by which this tree was
verified — only holds with that version.

## What was corrected, and how it is verified

The corrections are not guesses: each one starts from a measurement on the
factory binary and ends with a measurement on the device. Some of the main
ones:

| area | defect | confirming measurement |
|---|---|---|
| cameras | the IMX230 module was chosen without reading the EEPROM that tells the two suppliers apart: rotated image and wrong Bayer order | `/proc/driver/camera_info` identical line by line to the factory one |
| audio | nobody was turning the AW87329 amplifier on: `aw87329_audio_kspk` was compiled and unreachable | `hwen` goes from 0 to 1 on its own when Android plays |
| headphone jack | `CONFIG_ACCDET_EINT` turned on made the probe fail with `-ENODEV` | `/dev/accdet` with the same major/minor as stock |
| TEE | the SPI clock was not registered: 64 errors per boot and Trusted Applications not loaded | errors from 64 to 0, `tee_ta_load` as in stock |
| boot | `REVERSE_CHARGER` missing from the enum sent the phone into a bootloop | the phone boots |

## Status

Verified on the device: cameras (both), audio, jack, sensors (light,
proximity, accelerometer, magnetometer), touch, LEDs and torch, NFC,
fingerprint reader, wireless charging, TEE.

A paired stress test against the factory kernel — suspend and resume,
cameras, audio, graphics, storage, CPU load, network, three rounds each way —
produced no panics, BUGs, call traces or WARNINGs on either side, and the
remaining messages appear in the same orders of magnitude.

## Licence

GPL v2, like the Linux kernel. See [COPYING](COPYING).
