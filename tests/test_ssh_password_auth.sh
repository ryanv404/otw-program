#!/usr/bin/bash

RED="\e[1;31m"
GREEN="\e[1;32m"
CLRFMT="\e[0m"

# Clear prior stored data
[[ -f "data/otw_data.dat" ]] && rm data/otw_data.dat
[[ -f "data/known_otw_hosts.dat" ]] && rm data/known_otw_hosts.dat

# Save password for level and check authentication over SSH
bin/otw -s bandit0 bandit0 > /dev/null
TEST_OUTPUT=$(bin/otw bandit0 2>&1)

SUBSTR="Authenticated as bandit0"
echo "[*] Testing the ability to use a stored password to log in over SSH."
echo -n "bandit0		"
[[ $TEST_OUTPUT == *"$SUBSTR"* ]] && echo -e "${GREEN}[PASSED]${CLRFMT}" || echo -e "${RED}[FAILED]${CLRFMT}"

unset TEST_OUTPUT SUBSTR
