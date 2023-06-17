#!/usr/bin/bash

LEVELS="bandit12 narnia9 behemoth3 maze1 utumno5 vortex21 manpage0 drifter14 formulaone3 leviathan6 krypton2 natas21"

# Add passwords to various levels
for level in $LEVELS; do bin/otw -s $level $level > /dev/null; done; unset level;

# Display stored level data for the levels with passwords set
for level in $LEVELS; do bin/otw -l $level; done; unset level;
