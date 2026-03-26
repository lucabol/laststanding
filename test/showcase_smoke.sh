#!/bin/bash

set -u -o pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
FIXTURE_DIR="$SCRIPT_DIR/showcase_smoke"
BIN_DIR="$(cd "$SCRIPT_DIR/../bin" && pwd)"
SUFFIX="${1:-}"
if [ $# -gt 0 ]; then
  shift
fi
RUNNER=("$@")

TMP_DIR="$(mktemp -d)"
cleanup() {
  rm -rf "$TMP_DIR"
}
trap cleanup EXIT

# When cross-compiled binaries have a suffix (e.g. .armhf, .aarch64),
# create a symlink directory so the freestanding shell can find them by
# their unsuffixed names (e.g. "sort" → "sort.armhf").
SHELL_BIN_DIR="$BIN_DIR"
if [ -n "$SUFFIX" ]; then
  SHELL_BIN_DIR="$TMP_DIR/shell_bin"
  mkdir -p "$SHELL_BIN_DIR"
  for f in "$BIN_DIR"/*"$SUFFIX"; do
    base="$(basename "$f" "$SUFFIX")"
    ln -sf "$f" "$SHELL_BIN_DIR/$base"
  done
fi

run_target() {
  local exe="$1"
  shift
  if [ ${#RUNNER[@]} -gt 0 ]; then
    "${RUNNER[@]}" "$BIN_DIR/${exe}${SUFFIX}" "$@"
  else
    "$BIN_DIR/${exe}${SUFFIX}" "$@"
  fi
}

show_file() {
  sed -n 'l' "$1"
}

run_check() {
  local name="$1"
  local expected="$2"
  local expected_exit="$3"
  shift 3

  local safe_name="${name// /_}"
  safe_name="${safe_name//\//_}"
  local out="$TMP_DIR/${safe_name}.out"

  echo "--- Running $name ---"
  run_target "$@" >"$out"
  local status=$?
  if [ "$status" -ne "$expected_exit" ]; then
    echo "FAIL: $name exited with $status (expected $expected_exit)"
    return 1
  fi

  if ! cmp -s "$out" "$expected"; then
    echo "FAIL: $name output mismatch"
    echo "Expected:"
    show_file "$expected"
    echo "Actual:"
    show_file "$out"
    return 1
  fi
}

run_env_check() {
  local name="$1"
  local expected="$2"
  local expected_exit="$3"
  local env_name="$4"
  local env_value="$5"
  shift 5

  local safe_name="${name// /_}"
  safe_name="${safe_name//\//_}"
  local out="$TMP_DIR/${safe_name}.out"

  echo "--- Running $name ---"
  (
    export "$env_name=$env_value"
    run_target "$@" >"$out"
  )
  local status=$?
  if [ "$status" -ne "$expected_exit" ]; then
    echo "FAIL: $name exited with $status (expected $expected_exit)"
    return 1
  fi

  if ! cmp -s "$out" "$expected"; then
    echo "FAIL: $name output mismatch"
    echo "Expected:"
    show_file "$expected"
    echo "Actual:"
    show_file "$out"
    return 1
  fi
}

run_shell_redir_check() {
  local name="$1"
  local actual_name="$2"
  local script_text="$3"

  local safe_name="${name// /_}"
  safe_name="${safe_name//\//_}"
  local work_dir="$TMP_DIR/${safe_name}.work"
  local expected="$TMP_DIR/${safe_name}.expected"
  local shell_out="$TMP_DIR/${safe_name}.stdout"
  local shell_err="$TMP_DIR/${safe_name}.stderr"

  mkdir -p "$work_dir"
  cp "$FIXTURE_DIR/sample.txt" "$FIXTURE_DIR/sort_numeric.txt" "$work_dir"/
  cat "$FIXTURE_DIR/sort_numeric.expected" "$FIXTURE_DIR/upper.expected" >"$expected"

  echo "--- Running $name ---"
  (
    cd "$work_dir" || exit 1
    export PATH="$SHELL_BIN_DIR${PATH:+:$PATH}"
    printf '%s' "$script_text" | run_target sh >"$shell_out" 2>"$shell_err"
  )
  local status=$?
  if [ "$status" -ne 0 ]; then
    if [ -s "$shell_err" ]; then
      echo "stderr:"
      show_file "$shell_err"
    fi
    if [ -s "$shell_out" ]; then
      echo "stdout:"
      show_file "$shell_out"
    fi
    echo "FAIL: $name exited with $status (expected 0)"
    return 1
  fi

  local actual="$work_dir/$actual_name"
  if [ ! -f "$actual" ]; then
    if [ -s "$shell_err" ]; then
      echo "stderr:"
      show_file "$shell_err"
    fi
    echo "FAIL: $name did not create $actual_name"
    return 1
  fi

  if ! cmp -s "$actual" "$expected"; then
    echo "FAIL: $name output mismatch"
    echo "Expected:"
    show_file "$expected"
    echo "Actual:"
    show_file "$actual"
    if [ -s "$shell_err" ]; then
      echo "stderr:"
      show_file "$shell_err"
    fi
    return 1
  fi
}

run_shell_pipe_check() {
  local name="$1"
  local env_name="$2"
  local env_value="$3"
  local script_text="$4"
  local expected_stdout="$5"

  local safe_name="${name// /_}"
  safe_name="${safe_name//\//_}"
  local work_dir="$TMP_DIR/${safe_name}.work"
  local expected="$TMP_DIR/${safe_name}.expected"
  local shell_out="$TMP_DIR/${safe_name}.stdout"
  local shell_err="$TMP_DIR/${safe_name}.stderr"

  mkdir -p "$work_dir"
  printf '%s' "$expected_stdout" >"$expected"

  echo "--- Running $name ---"
  (
    cd "$work_dir" || exit 1
    export PATH="$SHELL_BIN_DIR${PATH:+:$PATH}"
    export "$env_name=$env_value"
    printf '%s' "$script_text" | run_target sh >"$shell_out" 2>"$shell_err"
  ) &
  local shell_pid=$!
  (
    sleep 15
    kill -KILL "$shell_pid" 2>/dev/null
  ) &
  local watchdog_pid=$!

  wait "$shell_pid"
  local status=$?
  kill "$watchdog_pid" 2>/dev/null
  wait "$watchdog_pid" 2>/dev/null

  if [ "$status" -eq 137 ]; then
    if [ -s "$shell_err" ]; then
      echo "stderr:"
      show_file "$shell_err"
    fi
    if [ -s "$shell_out" ]; then
      echo "stdout:"
      show_file "$shell_out"
    fi
    echo "FAIL: $name timed out (possible pipe hang)"
    return 1
  fi

  if [ "$status" -ne 0 ]; then
    if [ -s "$shell_err" ]; then
      echo "stderr:"
      show_file "$shell_err"
    fi
    if [ -s "$shell_out" ]; then
      echo "stdout:"
      show_file "$shell_out"
    fi
    echo "FAIL: $name exited with $status (expected 0)"
    return 1
  fi

  if ! cmp -s "$shell_out" "$expected"; then
    echo "FAIL: $name output mismatch"
    echo "Expected:"
    show_file "$expected"
    echo "Actual:"
    show_file "$shell_out"
    if [ -s "$shell_err" ]; then
      echo "stderr:"
      show_file "$shell_err"
    fi
    return 1
  fi
}

echo "=== Running showcase smoke tests ==="
pushd "$FIXTURE_DIR" >/dev/null || exit 1

run_check "base64" "base64.expected" 0 base64 sample.txt || exit 1
run_check "base64 -d" "sample.txt" 0 base64 -d sample.b64 || exit 1
run_check "checksum" "checksum.expected" 0 checksum sample.txt || exit 1
run_check "countlines" "countlines.expected" 3 countlines sample.txt || exit 1
run_check "grep" "grep.expected" 0 grep beta sample.txt || exit 1
run_check "hexdump" "hexdump.expected" 0 hexdump sample.txt || exit 1
run_check "ls" "ls.expected" 0 ls lsdir || exit 1
run_env_check "printenv" "printenv.expected" 0 LASTSTANDING_SMOKE showcase-value printenv LASTSTANDING_SMOKE || exit 1
run_check "sort -n" "sort_numeric.expected" 0 sort -n sort_numeric.txt || exit 1
run_check "sort -u" "sort_unique.expected" 0 sort -u sort_unique.txt || exit 1
run_check "upper" "upper.expected" 0 upper sample.txt || exit 1
run_check "wc" "wc.expected" 0 wc sample.txt || exit 1
run_check "led" "led.expected" 0 led || exit 1
run_check "sh --help" "sh.expected" 0 sh --help || exit 1
run_shell_redir_check "sh redirection" "sh-redir.out" $'sort -n < sort_numeric.txt > sh-redir.out\nupper sample.txt >> sh-redir.out\nexit 0\n' || exit 1
pipe_leaf="sh_pipe.work"
run_shell_pipe_check "sh pipe" "LASTSTANDING_SMOKE" "showcase-value" $'printenv LASTSTANDING_SMOKE | sort\nexit 0\n' "${pipe_leaf}\$ LASTSTANDING_SMOKE=showcase-value"$'\n'"${pipe_leaf}\$ " || exit 1

popd >/dev/null || exit 1
echo "=== Showcase smoke tests passed ==="
