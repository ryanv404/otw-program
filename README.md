# OTW Program

A program that is in development to make the process of connecting to [OverTheWire](https://overthewire.org/wargames/ "OTW") wargames quicker and more convenient.

## Planned Features

- Stores the level passwords you've uncovered locally.
- Connect to a wargame and start working by simply providing the level name (if password is stored).
- Display user's progress in completing all of the wargames.
- Uses libssh2 to connect to OTW wargames.

## Requirements

- [libssh2 C library](https://www.libssh2.org/ "libssh2")
- make
- gcc

## Building

Clone this git repository to your machine. Read through the source code so that you understand what you are installing :-). Then use `make` to build the program.
 
```bash
git clone https://github.com/ryanv404/otw-program.git
cd otw-program
./configure
make
```

## Usage

```
bin/otw [-h|--help] [-d|--display] [-c|--complete <level>] [-s|--store <password> <level>]
bin/otw <level-name>
```

## Examples

```bash
# Stores the password ("deadbeef") for the OTW
# bandit2 wargame to speed up future connections to the wargame.
bin/otw -s deadbeef bandit2

# Connects to the OTW bandit2 wargame over SSH.
bin/otw bandit2

# Displays the user's progress at completing all
# of the OTW wargames.
bin/otw -d
```
