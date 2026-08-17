#!/usr/bin/env bash
# Shared secret-detection logic used by pre-commit and pre-push hooks.
# Usage: secret_guard.sh <mode> <commit-or-range...>
#   mode = "diff"    -> args are extra args passed to `git diff --cached` (pre-commit)
#   mode = "commits" -> args are commit SHAs to inspect individually (pre-push)

set -euo pipefail

FILENAME_PATTERN='secrets\.ya?ml$'

# key: value  where value looks like a real secret, not a placeholder / !secret ref / empty string
CONTENT_PATTERN='^\+.*\b(wifi_password|api_password|api_encryption_key|mqtt_password|web_server_password|ota_password)\s*:\s*("?)[^"!'"'"'[:space:]]'
PLACEHOLDER_PATTERN='YourPassword|YourSSID|your_password|your_ssid|changeme|example|xxxxxxxx'

found=0

check_diff_text() {
  # $1 = full diff text (already captured, not read from a pipe -
  #      avoids the classic "stdin consumed by the first grep" bug)
  local diff_text="$1"
  local bad_files bad_content
  bad_files=$(printf '%s\n' "$diff_text" | command grep -E "^\+\+\+ b/.*$FILENAME_PATTERN" || true)
  bad_content=$(printf '%s\n' "$diff_text" | command grep -E "$CONTENT_PATTERN" | command grep -Ev "$PLACEHOLDER_PATTERN" || true)
  if [ -n "$bad_files" ] || [ -n "$bad_content" ]; then
    echo "  BLOQUEADO: contenido que parece un secreto real:"
    [ -n "$bad_files" ] && echo "$bad_files" | sed 's/^/    fichero: /'
    [ -n "$bad_content" ] && echo "$bad_content" | sed 's/^/    linea:   /'
    found=1
  fi
}

mode="$1"; shift

if [ "$mode" = "diff" ]; then
  diff_text="$(git diff --cached "$@")"
  check_diff_text "$diff_text"
elif [ "$mode" = "commits" ]; then
  for sha in "$@"; do
    echo "-- revisando commit $sha --"
    diff_text="$(git show "$sha")"
    check_diff_text "$diff_text"
  done
else
  echo "secret_guard.sh: modo desconocido '$mode'" >&2
  exit 2
fi

if [ "$found" -eq 1 ]; then
  echo
  echo "###########################################################"
  echo "# Commit/push BLOQUEADO: parece contener secrets.yaml o    #"
  echo "# una contraseña/clave real en texto plano.                #"
  echo "#                                                           #"
  echo "# - Si es secrets.yaml: quitalo del staging (git restore    #"
  echo "#   --staged secrets.yaml) - solo debe vivir en local.      #"
  echo "# - En los YAML de config usa siempre !secret nombre_clave  #"
  echo "#   en vez del valor real.                                  #"
  echo "# - Si es un falso positivo, usa --no-verify (con cuidado). #"
  echo "###########################################################"
  exit 1
fi

exit 0
