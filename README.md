Pi-FM-MPX
=========

*Documentation are being written*

## FM-MPX transmitter based on the Pi-FM-RDS project.

This program is a modified fork from the Pi-FM-RDS project by Christophe Jacquet.

Pi-FM-MPX allow you to pipe MPX audio and directly broadcasting it via RPi's PWM pin.

It is based on the FM transmitter created by [Oliver Mattos and Oskar Weigl](http://www.icrobotics.co.uk/wiki/index.php/Turning_the_Raspberry_Pi_Into_an_FM_Transmitter), and later adapted to using DMA by [Richard Hirst](https://github.com/richardghirst). Christophe Jacquet adapted it and added the RDS data generator and modulator. The transmitter uses the Raspberry Pi's PWM generator to produce VHF signals.

It is compatible with both the Raspberry Pi 1 (the original one) and the Raspberry Pi 2, 3 and 4.

Pi-FM-MPX has been developed for experimentation only. It is not a media center, it is not intended to broadcast music to your stereo system. See the [legal warning](#warning-and-disclaimer).

## How to use it?

Dependencies: 

- libsndfile
- libsoxr

It also depends on the Linux `rpi-mailbox` driver, so you need a recent Linux kernel. The Raspbian releases have this starting from August 2015.

**Important.** The binaries compiled for the Raspberry Pi 1 are not compatible with the Raspberry Pi 2/3, and conversely. Always re-compile when switching models, so do not skip the `make clean` step in the instructions below!

Clone the source repository and run `make` in the `src` directory:

```bash
git clone https://github.com/fluxth/pi-fm-mpx.git
cd pi-fm-mpx/src
make clean
make
```

**Important.** If `make` reports any error, then no `pi_fm_mpx` executable file is generated (and vice versa). So any error must be fixed before you can proceed to the next steps. `make` may fail if any required library is missing (see above), or it could be a bug on a specific/newer distribution. In this case, please file a bug.

If `make` reports no error (i.e. the `pi_fm_mpx` executable gets generated), you can then simply run:

```
sudo ./pi_fm_mpx
```

This will generate an FM transmission on 107.9 MHz, with default station name (PS), radiotext (RT) and PI-code, without audio. The radiofrequency signal is emitted on GPIO 4 (pin 7 on header P1).

The more general syntax for running Pi-FM-RDS is as follows:

```
pi_fm_rds [-freq freq] [-audio file] [-ppm ppm_error] [-pi pi_code] [-ps ps_text] [-rt rt_text]
```

All arguments are optional:

* `-freq` specifies the carrier frequency (in MHz). Example: `-freq 107.9`.
* `-audio` specifies an audio file to play as audio. The sample rate does not matter: Pi-FM-RDS will resample and filter it. If a stereo file is provided, Pi-FM-RDS will produce an FM-Stereo signal. Example: `-audio sound.wav`. The supported formats depend on `libsndfile`. This includes WAV and Ogg/Vorbis (among others) but not MP3. Specify `-` as the file name to read audio data on standard input (useful for piping audio into Pi-FM-RDS, see below).
* `-pi` specifies the PI-code of the RDS broadcast. 4 hexadecimal digits. Example: `-pi FFFF`.
* `-ps` specifies the station name (Program Service name, PS) of the RDS broadcast. Limit: 8 characters. Example: `-ps RASP-PI`.
* `-rt` specifies the radiotext (RT) to be transmitted. Limit: 64 characters. Example: `-rt 'Hello, world!'`.
* `-ctl` specifies a named pipe (FIFO) to use as a control channel to change PS and RT at run-time (see below).
* `-ppm` specifies your Raspberry Pi's oscillator error in parts per million (ppm), see below.

By default the PS changes back and forth between `Pi-FmRds` and a sequence number, starting at `00000000`. The PS changes around one time per second.

### Piping audio into Pi-FM-RDS

If you use the argument `-audio -`, Pi-FM-RDS reads audio data on standard input. This allows you to pipe the output of a program into Pi-FM-RDS. For instance, this can be used to read MP3 files using Sox:

```
sox -t mp3 http://www.linuxvoice.com/episodes/lv_s02e01.mp3 -t wav -  | sudo ./pi_fm_rds -audio -
```

Or to pipe the AUX input of a sound card into Pi-FM-RDS:

```
sudo arecord -fS16_LE -r 44100 -Dplughw:1,0 -c 2 -  | sudo ./pi_fm_rds -audio -
```

## Warning and Disclaimer

PiFmRds is an **experimental** program, designed **only for experimentation**. It is in no way intended to become a personal *media center* or a tool to operate a *radio station*, or even broadcast sound to one's own stereo system.

In most countries, transmitting radio waves without a state-issued licence specific to the transmission modalities (frequency, power, bandwidth, etc.) is **illegal**.

Therefore, always connect a shielded transmission line from the RaspberryPi directly
to a radio receiver, so as **not** to emit radio waves. Never use an antenna.

Even if you are a licensed amateur radio operator, using PiFmRds to transmit radio waves on ham frequencies without any filtering between the RaspberryPi and an antenna is most probably illegal because the square-wave carrier is very rich in harmonics, so the bandwidth requirements are likely not met.

I could not be held liable for any misuse of your own Raspberry Pi. Any experiment is made under your own responsibility.

## Licenses

This program contains code from the original PiFmRds project which is licensed under GNU GPL v3.

Therefore, this program shall use and contain the same license.

For more info, visit the original project at [https://github.com/ChristopheJacquet/PiFmRds](https://github.com/ChristopheJacquet/PiFmRds).

--------

© 2020 fluxth. Released under the GNU GPL v3.
