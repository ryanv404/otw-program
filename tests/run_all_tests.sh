#!/usr/bin/bash

echo -e "[+] Starting testing...\n"

echo "[PASSWORD STORAGE TESTING]"
echo "**************************"
[[ -f "data/otw-data.dat" ]] && rm data/otw-data.dat
[[ -f "tests/test_add_some_passwords.sh" ]] && tests/test_add_some_passwords.sh

echo -e "\n[PROGRESS TRACKING TESTING]"
echo "***************************"
[[ -f "data/otw-data.dat" ]] && rm data/otw-data.dat
[[ -f "tests/test_complete_some_levels.sh" ]] && tests/test_complete_some_levels.sh

[[ -f "data/otw-data.dat" ]] && rm data/otw-data.dat
[[ -f "tests/test_complete_all_levels.sh" ]] && tests/test_complete_all_levels.sh

echo -e "\n[+] Done with testing."
