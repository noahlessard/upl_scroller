
## Setup on Raspberry Pi (Raspbian)

```bash
sudo apt-get update
sudo apt-get install cmake g++ libnotcurses-dev

cmake -B build
make -C build
./upl_scroller
```
