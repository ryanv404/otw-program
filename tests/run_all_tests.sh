#!/usr/bin/bash

echo "[+] Starting tests."
[[ -f "data/otw-data.dat" ]] && rm data/otw-data.dat
[[ -f "tests/test_add_passwords.sh" ]] && tests/test_add_passwords.sh

[[ -f "data/otw-data.dat" ]] && rm data/otw-data.dat
[[ -f "tests/test_complete_some_levels.sh" ]] && tests/test_complete_some_levels.sh

[[ -f "data/otw-data.dat" ]] && rm data/otw-data.dat
[[ -f "tests/test_complete_all_levels.sh" ]] && tests/test_complete_all_levels.sh

echo "[+] Done with tests."
