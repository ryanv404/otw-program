#!/usr/bin/env bash
#
# Bash script that connects to OTW levels via SSH.
# Usage: ./otw.sh <level>
# By: ryanv404, 2023.

LNAMES=(bandit natas leviathan krypton narnia behemoth utumno maze vortex manpage drifter formulaone)
PORTS=(2220 0 2223 2231 2226 2221 2227 2225 2228 2224 2230 2223)
MAXLVLS=(34 34 7 7 9 8 8 9 27 7 15 6)

check_usage() {
  if [[ $# -ne 1 ]]; then
    echo -e "usage: $0 <level>"
    exit 1
  fi
}

split_arg() {
  if [[ $1 =~ ^([a-zA-Z]+)([0-9]+)$ ]]; then
    name="${BASH_REMATCH[1]}"
    num="${BASH_REMATCH[2]}"
  else
    echo "[-] invalid level argument."
    exit 2
  fi
}

validate_level_name() {
  local lvlname=$1
  local idx=0

  for lname in "${LNAMES[@]}"; do
    if [[ "${lvlname}" == "${lname}" ]]; then
      port="${PORTS[$idx]}"
      max="${MAXLVLS[$idx]}"
      return
    fi
    (( idx++ ))
  done

  echo "[-] invalid level name."
  exit 2
}

validate_level_num() {
  local lnum=$1
  local lmax=$2

  if (( lnum < 0 || lnum > lmax )); then
    echo "[-] invalid level number."
    exit 3
  fi
}

main() {
  local name=""
  local num=""
  local max=""
  local port=""

  check_usage $@
  split_arg $@
  validate_level_name "${name}"
  validate_level_num "${num}" "${max}"

  local remotehost="${name}.labs.overthewire.org"
  echo "[*] Connecting to ${remotehost} as user ${name}${num}..."

  ssh -p "${port}" "${name}${num}@${remotehost}"

  exit 0
}

main $@
