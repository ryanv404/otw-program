#!/usr/bin/bash

[[ -f "data/otw_data.dat" ]] && rm data/otw_data.dat

echo -e "\n[*] Marking all levels as complete and testing progress table appearance."

# Test marking each level as complete
for i in {0..8}; do bin/otw -c "behemoth$i" > /dev/null; done; unset i;
for i in {0..9}; do bin/otw -c "maze$i" > /dev/null; done; unset i;
for i in {0..34}; do bin/otw -c "bandit$i" > /dev/null; done; unset i;
for i in {0..34}; do bin/otw -c "natas$i" > /dev/null; done; unset i;
for i in {0..8}; do bin/otw -c "utumno$i" > /dev/null; done; unset i;
for i in {0..27}; do bin/otw -c "vortex$i" > /dev/null; done; unset i;
for i in {0..7}; do bin/otw -c "manpage$i" > /dev/null; done; unset i;
for i in {0..15}; do bin/otw -c "drifter$i" > /dev/null; done; unset i;
for i in {0..6}; do bin/otw -c "formulaone$i" > /dev/null; done; unset i;
for i in {0..7}; do bin/otw -c "leviathan$i" > /dev/null; done; unset i;
for i in {0..7}; do bin/otw -c "krypton$i" > /dev/null; done; unset i;
for i in {0..9}; do bin/otw -c "narnia$i" > /dev/null; done; unset i;

# Display progress table
bin/otw -p
