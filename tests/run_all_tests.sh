#!/usr/bin/bash

RED="\e[1;31m"
PURP="\e[1;35m"
BLUE="\e[1;34m"
CLRFMT="\e[0m"

echo -e "${PURP}[+]${CLRFMT} Starting testing...\n"

echo -e "${BLUE}[PASSWORD STORAGE TESTING]${CLRFMT}"
echo -e "${BLUE}**************************${CLRFMT}"
[[ -f "tests/test_add_some_passwords.sh" ]] && tests/test_add_some_passwords.sh || echo -e "${RED}[MISSING]${CLRFMT}"

echo -e "\n${BLUE}[SSH PASSWORD AUTHENTICATION TESTING]${CLRFMT}"
echo -e "${BLUE}******************************************${CLRFMT}"
[[ -f "tests/test_ssh_password_auth.sh" ]] && tests/test_ssh_password_auth.sh || echo -e "${RED}[MISSING]${CLRFMT}"

echo -e "\n${BLUE}[PROGRESS TRACKING TESTING]${CLRFMT}"
echo -e "${BLUE}********************************${CLRFMT}"
[[ -f "tests/test_complete_some_levels.sh" ]] && tests/test_complete_some_levels.sh || echo -e "${RED}[MISSING]${CLRFMT}"
[[ -f "tests/test_complete_all_levels.sh" ]] && tests/test_complete_all_levels.sh || echo -e "${RED}[MISSING]${CLRFMT}"

echo -e "${PURP}[+]${CLRFMT} Done with testing."
