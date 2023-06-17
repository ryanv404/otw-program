#!/usr/bin/bash

LEVELS="bandit12 narnia9 behemoth3 maze1 utumno5 vortex21 manpage0 drifter14 formulaone3 leviathan6 krypton2 natas21"
PW_ARR=()
LVL_ARR=()

# Add passwords to various levels
for level in $LEVELS; do
	password=$(echo $RANDOM | md5sum | head -c20)
	bin/otw -s $password $level > /dev/null
	PW_ARR+=($password)
	LVL_ARR+=($level)
done

OUT_ARR=()
# Display stored level data for the levels with passwords set
for level in $LEVELS; do
	OUTPUT=$(bin/otw -l $level | grep -E '^password' | sed 's/password\.\{1,\}//g' | tr -d '\n')
	OUT_ARR+=($OUTPUT)
done
unset level

echo "[+] Testing whether input password and saved password is the same."
for i in {0..11}; do
	echo -ne "${LVL_ARR[$i]}   \t"
	[[ "${PW_ARR[$i]}" = "${OUT_ARR[$i]}" ]] && echo "  [PASSED]" || echo "  [FAILED]"
done
unset i
