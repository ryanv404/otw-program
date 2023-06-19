# OTW Program (in-progress)

A command line tool written in C that will make the process of connecting
to [OverTheWire](https://overthewire.org/wargames/ "OTW") levels quicker and
more convenient, keep track of passwords the user has uncovered, and track the
user's overall progress at completing the 183+ OTW wargames.

## Planned Features

- Stores the level passwords you've uncovered in a local file.
- Connect to a level and start working by simply providing the level
  name (if the level's password has been stored).
- Mark levels as completed from the command line; setting a password
  for a level automatically marks the previous level as completed.
- Display your progress at completing all of the wargames.
- Uses libssh to connect to OTW levels via SSHv2.

## Requirements

- [libssh C library](https://www.libssh.org/ "libssh")
- make
- gcc

## Building

Clone this git repository to your machine.  Then use `make` to build the
program.

```bash
git clone https://github.com/ryanv404/otw-program.git
cd otw-program
make
```

## Usage

```
usage: otw LEVEL
       otw [-h|--help] [-p|--progress] [-c|--complete LEVEL]
           [-s|--store PASSWORD LEVEL]

use `otw -h` or `otw --help` for more help.
```

## Examples

```bash
# Prompts for password and then connects to the OTW bandit21
# level over SSH.
otw bandit21

# Stores the password ("deadbeef") for the OTW bandit21 level
# to make future connections to the level more convenient.
otw -s deadbeef bandit21  <-- Do this once.

otw bandit21  <-------------- And from now on this will
							  connect automatically to
							  the level.

# Displays the user's progress at completing all
# of the OTW levels.
otw -p
```
