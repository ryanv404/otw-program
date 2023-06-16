# OTW Program (not finished)

A command line tool written in C that will make the process of connecting to [OverTheWire](https://overthewire.org/wargames/ "OTW") levels quicker and more convenient, keep track of passwords the user has uncovered, and track the user's overall progress at completing the 183+ OTW wargames.

## Planned Features

- Stores the level passwords you've uncovered in a local file.
- Connect to a wargame and start working by simply providing the level name (if password is stored).
- Mark levels as completed from the command line; setting a password for a level automatically marks it as completed.
- Display your progress at completing all of the wargames.
- Uses libssh2 to connect to OTW levels.

## Requirements

- [libssh2 C library](https://www.libssh2.org/ "libssh2")
- make
- gcc

## Building

Clone this git repository to your machine. Read through the source code so that you understand what you are installing :-). Then use `make` (or your favorite build system) to build the program.
 
```bash
git clone https://github.com/ryanv404/otw-program.git
cd otw-program
./configure
make
```

## Usage

```
Usage: otw LEVEL
       otw [-h|-p] [-c LEVEL]
       otw -s PASSWORD LEVEL

Connect to the OTW level LEVEL with `otw LEVEL`.

    LEVEL
        A single word composed of the wargame's name (e.g. "bandit")
        and the level number (e.g. "2").
        For example, `otw bandit2`, `otw vortex12`, etc.

    -c, --complete LEVEL
        Mark the level LEVEL as complete. Note: storing a password for a
        level automatically marks the level before it complete.

    -h, --help
        Show this help information.

    -p, --progress
        Display which wargames you've completed and your overall progress.

    -s, --store PASSWORD LEVEL
        Store password PASSWORD for the level LEVEL in a local file to
        make connecting to the level more convenient. Note: this will also
        mark the level before it (where the password was obtained) complete.
```

## Examples

```bash
# Connects to the OTW bandit21 wargame over SSH.
otw bandit21

# Stores the password ("deadbeef") for the OTW bandit21 wargame
# to make future connections to the wargame more convenient.
otw --store deadbeef bandit21

# Displays the user's progress at completing all
# of the OTW wargames.
otw --progress
```
