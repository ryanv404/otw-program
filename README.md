# OTW Program

A program that is in development to make the process of connecting to [OverTheWire](https://overthewire.org/wargames/ "OTW") wargames quicker and more convenient.

## Planned Features

- Connect to level and start working by simply providing the level name to the program.
- Allows you to store the level passwords you've uncovered.
- Otherwise, you can just provide the password at connection time.
- Display user's progress.
- Monitors OTW website for changes to the host names, ports, and available levels that are in use once per month, if desired.
- Uses libssh2 to connect to OTW levels with SSH version 2.

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
bin/otw [-h|--help] [-u|--user] [-s|--store <level-password>] <level-name>
```

## Examples

```bash
# Stores the password ("deadbeef") for the OTW
# bandit 2 wargame to speed up connection to the level.
bin/otw -s deadbeef bandit2

# Connects to the OTW bandit 2 wargame over SSH.
bin/otw bandit2

# Displays the user's progress at completing all
# of the OTW wargames.
bin/otw -u
```
