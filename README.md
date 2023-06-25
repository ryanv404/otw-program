# OTW Program (in-progress)

A command line tool written in C that makes the process of connecting
to [OverTheWire](https://overthewire.org/wargames/ "OTW") levels quicker and
more convenient, keeps track of level passwords the user has uncovered, and 
tracks the user's overall progress at completing the 183+ OTW levels.

## Planned Features

- Stores the level passwords you've uncovered in a local file.
- Connect to a level and start working by simply providing the level
  name (if the level's password has been stored; otherwise just enter it
  during the connection process as per usual).
- Mark levels as completed from the command line; setting a password
  for a level automatically marks the previous level as complete since that
  is the only place you could've gotten the password from ;-).
- Display your progress at completing all of the wargames at any time.

## In-Progress

- Currently testing libssh vs libssh2 (see the source tree above) for connecting
  to OTW levels via SSHv2.
- Refining the polling loop as well as the local and remote pseudoterminal settings
  after connection so that the keystroke delay is less noticeable.

## Requirements

- make
- gcc
- [libssh C library](https://www.libssh.org/ "libssh") or [libssh2 C library](https://www.libssh2.org/ "libssh2")

## Building

Clone this git repository to your machine. `cd` into either the libssh or libssh2
folder depending on which implementation you're interested in. Then, use `make` to
build the program.

```bash
git clone https://github.com/ryanv404/otw-program.git
cd otw-program/libssh  # or, cd otw-program/libssh2
make
```

## Usage

```
usage: otw LEVEL
       otw [-h|--help] [-p|--progress] [-c|--complete LEVEL]
           [-s|--store PASSWORD LEVEL]

use `otw -h` for more help.
```

## Examples

```bash
# Connects to the OTW bandit 21 level via SSH, prompting for the
# level's password during the connection process.
otw bandit21

# Stores the password ("deadbeef") for the OTW bandit 21 level so
# that connecting to the level does not prompt for a password.
otw -s deadbeef bandit21  <-- Do this once.

otw bandit21  <-------------- And from now on this will
                              connect automatically to
			      the level.

# Displays the user's progress at completing all
# of the OTW levels.
otw -p
```
