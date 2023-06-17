#!/usr/bin/bash

NUM_MARKED=0
NUM_EXPECTED=()
LVL_NAME=()

# Test marking each level as complete
for i in {21..24}; do bin/otw -c "bandit$i" > /dev/null; ((NUM_MARKED++)); done;
NUM_EXPECTED+=($NUM_MARKED); NUM_MARKED=0; LVL_NAME+=("bandit$i");
for i in {17..19}; do bin/otw -c "natas$i" > /dev/null; ((NUM_MARKED++)); done;
NUM_EXPECTED+=($NUM_MARKED); NUM_MARKED=0; LVL_NAME+=("natas$i");
for i in {3..5}; do bin/otw -c "leviathan$i" > /dev/null; ((NUM_MARKED++)); done;
NUM_EXPECTED+=($NUM_MARKED); NUM_MARKED=0; LVL_NAME+=("leviathan$i");
for i in {0..2}; do bin/otw -c "krypton$i" > /dev/null; ((NUM_MARKED++)); done;
NUM_EXPECTED+=($NUM_MARKED); NUM_MARKED=0; LVL_NAME+=("krypton$i");
for i in {3..5}; do bin/otw -c "narnia$i" > /dev/null; ((NUM_MARKED++)); done;
NUM_EXPECTED+=($NUM_MARKED); NUM_MARKED=0 LVL_NAME+=("narnia$i");
for i in {2..4}; do bin/otw -c "behemoth$i" > /dev/null; ((NUM_MARKED++)); done;
NUM_EXPECTED+=($NUM_MARKED); NUM_MARKED=0; LVL_NAME+=("behemoth$i");
for i in {2..6}; do bin/otw -c "utumno$i" > /dev/null; ((NUM_MARKED++)); done;
NUM_EXPECTED+=($NUM_MARKED); NUM_MARKED=0; LVL_NAME+=("utumno$i");
for i in {6..8}; do bin/otw -c "maze$i" > /dev/null; ((NUM_MARKED++)); done;
NUM_EXPECTED+=($NUM_MARKED); NUM_MARKED=0; LVL_NAME+=("maze$i");
for i in {21..27}; do bin/otw -c "vortex$i" > /dev/null; ((NUM_MARKED++)); done;
NUM_EXPECTED+=($NUM_MARKED); NUM_MARKED=0; LVL_NAME+=("vortex$i");
for i in {3..5}; do bin/otw -c "manpage$i" > /dev/null; ((NUM_MARKED++)); done;
NUM_EXPECTED+=($NUM_MARKED); NUM_MARKED=0; LVL_NAME+=("manpage$i");
for i in {12..13}; do bin/otw -c "drifter$i" > /dev/null; ((NUM_MARKED++)); done;
NUM_EXPECTED+=($NUM_MARKED); NUM_MARKED=0; LVL_NAME+=("drifter$i");
for i in {1..3}; do bin/otw -c "formulaone$i" > /dev/null; ((NUM_MARKED++)); done;
NUM_EXPECTED+=($NUM_MARKED); LVL_NAME+=("formulaone$i");

unset i NUM_MARKED

OUT_ARR=$(bin/otw -p | grep -E '^\[\w+' | sed 's/\[[[:alpha:]]\{1,\}\][[:space:]]\{1,\}//g' | sed 's/\/.\{1,\}//g' | tr '\n' ' ')

echo "[+] Testing whether the number of levels marked complete is the same as the number saved as complete."

i=0
for output in ${OUT_ARR[@]}; do
	echo -ne "${LVL_NAME[$i]}   \t"
	[[ "$output" -eq "${NUM_EXPECTED[$i]}" ]] && echo "[PASSED]" || echo "[FAILED]"
	((i++))
done
