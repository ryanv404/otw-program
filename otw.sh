#!/usr/bin/env bash
#########################################################
# Bash script that connects to OTW levels using Expect.
# Usage: ./otw.sh <LEVEL>
# By: ryanv404, 2023.
#########################################################

### GLOBALS ###
SCRIPT_NAME="$0"
ARGCOUNT="$#"
PASS_FILE_NAME="otw_passwords.txt"
PASS_DIR="${HOME}/.otw"
PASS_FILE_PATH="${PASS_DIR}/${PASS_FILE_NAME}"
LEVEL="$1"
LEVEL_NAME=""
LEVEL_PASS=""
RHOST=""
RPORT=""

### LEVELS DATA ###
declare -A LEVELS=(
  [bandit]="2220 34"
  [natas]="0000 34"
  [leviathan]="2223 7"
  [krypton]="2231 7"
  [narnia]="2226 9"
  [behemoth]="2221 8"
  [utumno]="2227 8"
  [maze]="2225 9"
  [vortex]="2228 27"
  [manpage]="2224 7"
  [drifter]="2230 15"
  [formulaone]="2223 6"
)

is_correct_usage() {
  if (( ARGCOUNT != 1 )); then
    echo "[-] usage: ${SCRIPT_NAME} <LEVEL>"
    return 1
  fi
  return 0
}

has_dependencies() {
  local rv=0
  local dep=""
  local cmd=""

  [ ! -x "$(command -v ssh 2>/dev/null)" ] && rv=1
  [ ! -x "$(command -v expect 2>/dev/null)" ] && (( rv = rv ? 3 : 2 ))

  case "${rv}" in
    0) return 0;;
    1) dep="dependency"; cmd="\`ssh\` command is";;
    2) dep="dependency"; cmd="\`expect\` command is";;
    3) dep="dependencies"; cmd="\`ssh\` and \`expect\` commands are";;
  esac

  echo "[-] Missing ${dep}. The ${cmd} required but could not be found on your system."
  return 1
}

split_level_arg() {
  # Level name length limits: maze (4 chars), formulaone (10 chars)
  level_regex='^([a-zA-Z]{4,10})([1-9]?[0-9])$'
  if [[ ${LEVEL} =~ ${level_regex} ]]; then
    LEVEL_NAME="${BASH_REMATCH[1]}"
    level_num="${BASH_REMATCH[2]}"
    return 0
  else
    echo "[-] Valid OTW levels are: bandit, natas, leviathan, krypton, narnia,"
    echo "    behemoth, utumno, maze, vortex, manpage, drifter, and formulaone."
    return 1
  fi
}

validate_level() {
  local rv=0
  local level_num=""
  local max_level=""
  local levelArr=()

  # Split level_arg into level_name and level_num
  split_level_arg || return 1

  # Validate level_name
  for lname in "${!LEVELS[@]}"; do
    if [[ "${LEVEL_NAME}" == "${lname}" ]]; then
      rv=1
      break
    fi
  done

  if (( rv == 0 )); then
    echo "[-] Valid OTW levels are: bandit, natas, leviathan, krypton, narnia,"
    echo "    behemoth, utumno, maze, vortex, manpage, drifter, and formulaone."
    return 1
  fi

  # Validate level_num
  levelArr=(${LEVELS[${LEVEL_NAME}]})
  max_level="${levelArr[1]}"
  if (( level_num < 0 || level_num > max_level )); then
    echo "[-] Valid ${LEVEL_NAME} levels are: ${LEVEL_NAME}0 - ${LEVEL_NAME}${max_level}."
    return 1
  fi
  return 0
}

create_password_file() {
  local actual_file_path=""
  local max_level=""
  local levelArr=()

  if [[ -d "${PASS_DIR}" ]]; then
    touch "${PASS_FILE_PATH}" || return 1
    actual_file_path="${PASS_FILE_PATH}"
  elif [[ -d "${HOME}" ]]; then
    mkdir -p "${PASS_DIR}" || return 1
    touch "${PASS_FILE_PATH}" || return 1
    actual_file_path="${PASS_FILE_PATH}"
  else
    touch "${PASS_FILE_NAME}" || return 1
    actual_file_path="${PASS_FILE_NAME}"
  fi

  echo "[+] Created a password file at ${actual_file_path}"

  # Write default data into file if it exists and has a size of 0 bytes
  if [[ -f "${actual_file_path}" && ! -s "${actual_file_path}" ]]; then
    for lname in "${!LEVELS[@]}"; do
      levelArr=(${LEVELS[${lname}]})
      max_level="${levelArr[1]}"

      for (( i = 0; i <= max_level; i++ )); do
        if [[ "${lname}${i}" == "bandit0" ]]; then
          echo "${lname}${i}:bandit0" >> "${actual_file_path}"
          continue
        fi
        echo "${lname}${i}:x" >> "${actual_file_path}"
      done
    done
  fi

  return 0
}

get_saved_password() {
  local actual_file_path="$1"

  while IFS=":" read -r lvl passwd; do
    if [[ "${LEVEL}" == "${lvl}" ]]; then
      LEVEL_PASS="${passwd}"
      break
    fi
  done < "${actual_file_path}"

  return 0
}

get_level_password() {
  if [[ -r "${PASS_FILE_PATH}" ]]; then
    # Check default location
    get_saved_password "${PASS_FILE_PATH}"
  elif [[ -r "./${PASS_FILE_NAME}" ]]; then
    # Try current directory
    get_saved_password "./${PASS_FILE_NAME}"
  else
    # No password file found so create one
    echo "[*] No OTW passwords file found."
    if ! create_password_file; then
      echo "[-] Unable to create an OTW passwords file."
    fi
  fi

  return 0
}

handle_natas_levels() {
  echo "${LEVEL} is a natas level and the pass is ${LEVEL_PASS}"
  return 0
}

### RUN EXPECT COMMANDS ###
connect_with_expect() {
  expect -c "set LEVEL ${1}; set RHOST ${2}; set RPORT ${3}; set LEVEL_PASS ${4};" -c '
  log_user 0
  match_max 2000
  set timeout 20

  send_user -- "\[\*\] Connecting to ${RHOST} as ${LEVEL}...\n"
  spawn ssh -p ${RPORT} "${LEVEL}@${RHOST}"

  expect {
    # Accept host key if this is the first connection
    -re "fingerprint.* $" {
      send -- "yes\r"
      exp_continue
    }
    # Allow user to manually enter password if saved password is incorrect
    -re ".*please try again.*" {
      send_user -- "\[-\] The password was incorrect. Switching to interactive.\n"
      send -- "\r"
      interact
    }
    # Enter level password
    -re ".*password: $" {
      send_user -- "\[\*\] Authenticating...\n"
      send -- "${LEVEL_PASS}\r"
      exp_continue
    }
    # Switch control to user once we receive a shell prompt
    ".*\$ $" {
      send_user -- "\[+\] Successfully logged in to ${LEVEL}\n"
      send -- "\r"
      interact
    }
    timeout { send_user -- "\[-\] The program timed out.\n"; return 1; }
    eof { send_user -- "\[-\] The program received EOF.\n"; return 1; }
  }
'
}

### MAIN FUNCTION ###
main() {
  # Validate user input and existence of program dependencies (ssh & Expect)
  is_correct_usage || exit 1
  has_dependencies || exit 1
  validate_level   || exit 1

  get_level_password

  if [[ "${LEVEL_NAME}" == "natas" ]]; then
    # Natas levels are accessed with a web browser
    handle_natas_levels
  else
    # Connect to all other levels via Expect + ssh
    local levelArr=(${LEVELS[${LEVEL_NAME}]})
    RPORT="${levelArr[0]}"
    RHOST="${LEVEL_NAME}.labs.overthewire.org"
    connect_with_expect "${LEVEL}" "${RHOST}" "${RPORT}" "${LEVEL_PASS}" || exit 1
  fi

  exit 0
}

### RUN MAIN PROGRAM ###
main "$@"
