![OpenIPC logo][logo]

## OpenIPC WiFiLINK Builder for Japanese Users
**_Experimental system for building OpenIPC firmware for RunCam WiFiLINK devices_**
- **_[Current release of firmware builds](https://github.com/OpenIPC/builder/releases/tag/latest)_**


### Specialized features

- Tweaker for automatically configuring devices according to profile (gpio, wifi, etc).
- Specialized _[storage location](https://github.com/OpenIPC/builder/releases/tag/latest)_ for customized firmware for well-known devices.
- QR code recognition to automatically _[connect to WiFi](https://openipc.org/tools/qr-code-generator)_ on your home network.
- Change default WiFi channel to 36, japanese users can use this channel without DFS/TPC. But remember, OUTDOOR USE IS PROHIBITED.
- Support RTL8821A based USE dongles which are commonly selled in Japanese market. You can only use MIC(TELEC) certified device in Japan.
  - tp-link Archer T2U Nano(TELEC certified version) is tested. This dongle is very small and has less Tx power, but it's enough for indoor use.

### List of known and supported devices

```
RunCam WiFiLink          SSC338Q      IMX415                     NOR_16M   done
```


### Device setup

#### WiFi Settings
Put user.ini onto your SD card. See RunCam's users manual for details.


### Preparing and using the project

```
sudo apt-get update -y
sudo apt-get install -y automake autotools-dev bc build-essential curl fzf git libtool rsync \
  unzip mc tree python-is-python3
git clone https://github.com/openipc/builder.git
cd builder
./build-wifilinkjp.sh
```

build-wifilinkjp.sh creates WiFiLink-part0.bin and WiFiLink-part1.bin in builder folder. Copy those to your SD card, and reboot WiFILINK board. Then, the board update firmware automatically. Don't power off during LED brinking.

### Existing problems

- Tx power level is not tweaked. DON'T USE THIS FIRMWARE FOR OUTDOOR LONG RANGE FLIGHTS.
- DFS is not disabled. VIDEO STREAM CAN BE INTERRUPTED BY DFS. DON'T FLY NEARBY HUMANS, ANIMALS, OR ANY CRASHABLE MATERIALS.

### Additional information

- USB WiFi driver is replaced by aircrack-ng's repository. This enables RTL8814A based devices and RTL8821A based devices. RTL8814A is not tested.
- You can replace out of box WiFi module. To replace it, you need to DYI JST-SH 6-pins(1mm pitch) connector to USB Type-A Female adaptor.

### Technical support and donations

If you have any troubles, don't ask OpenIPC guys. This repository isn't supported officialy.
Donations for OpenIPC is recommended. Hopefully, they can make more elegant solution.

Please **_[support our project](https://openipc.org/support-open-source)_** with donations or orders for development or maintenance. Thank you!

[logo]: https://openipc.org/assets/openipc-logo-black.svg
