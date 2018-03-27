# WriterPi

Picture

## What's this?

Typewriter of the future.


## Part List

- A Raspberry Pi. Tested on 3B but should work as well on previous iterations (~40€)
- A Raspberry pi compatible E-Ink screen. We mainly targeted waveshare/good display 7.5" and 4.2" screens so far. (~40€) ** /!\ We did not manage to implement partial refresh on the 7.5" screen so far. The refresh rate on this screen is REALLY slow. **
- A 61 keys Mechanical Keyboard. (Chinese bootlegs are ~70€)
- A generic 3.7V 2500mah phone battery. (~10€)
- Battery charger/regulator. I use the [Adafruit powerboost 500c](https://www.adafruit.com/product/1944). If you live in the EU, you should probably buy it from a local reseller to avoid import taxes. [Pimoroni](https://shop.pimoroni.com/products/powerboost-500-charger-rechargeable-5v-lipo-usb-boost-500ma) is quite nice (unpaid endorsement, plz Pimoroni, send free stuff!). You can probably find some cheaper Chinese bootlegs.
- Stuff to make the case. (Can't say, depends a lot of the kind of wood you'll use).
- Intense procrastination. (free)

Depending on where you buy those part and how much time you spend sourcing them, the final price tag will range between 100 and 200€.

## Build and Install

First, [install raspbian](https://www.raspberrypi.org/downloads/raspbian/) on your raspberrypi.

Then, install the necessary dependencies:

1. Download and install [bcm2835](http://www.airspayce.com/mikem/bcm2835/).
1. Activate the SPI port of your raspberry pi.
1. Build and Install [logkeys](https://github.com/kernc/logkeys/blob/master/INSTALL).

Then, open a terminal on the raspberry pi:

```
sudo apt-get install build-essential git
git clone $GITHUB_URL && cd writerpi
make
sudo make install
```

If, for some reason, you want to uninstall the software:

```
sudo make uninstall
```

You can then start the writer-pi service (start a root shell on the e-ink screen):

```
sudo systemctl start writerpi
```

If you want the service to start on boot:

```
sudo systemctl enable writerpi
```

## Troubleshooting

Open an issue on this GitHub repo.

## Roadmap

This project is not fully done yet, I would like to implement:

- UTF-8 support.
- 7.5" screen partial refresh.
- Get rid of Logkeys by parsing directly /dev/input events and applying the appropriate keymap.
- Run the terminal emulator in an unprivileged environment.
- Get a more reproducible case design (custom keyboard, integrated board design?). 
- Build and sell a small batch of these?

## Special Thanks

- Suckless team: we use a hacked version of [ST](https://st.suckless.org/) for terminal emulation. (Note: the Suckless team do not endorse nor is related to this project, we just like their work)
