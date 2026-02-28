#!/usr/bin/env bash
set -euo pipefail

usage() {
    cat <<'EOF'
Usage: tools/build-example.sh --example <name> [options]

Compiles one example by injecting EXAMPLE_* / TEST_* defines through CMake.

Options:
  -l, --list                  List all available EXAMPLE_* and their TEST_* macros.
      --list-tests <name>     List TEST_* macros for one example (e.g. fmac, EXAMPLE_FMAC).
  -e, --example <name>        Example name (e.g. fmac, adc, EXAMPLE_FMAC).
  -t, --test <id|macro>       Test selector (default: TEST_0). Accepts 0, 1, TEST_1...
      --no-test               Do not define any TEST_* macro.
  -p, --preset <preset>       CMake configure/build preset
                              (default: board-debug-eth-ksz8041).
  -b, --board-name <name>     Optional BOARD_NAME override (e.g. TEST).
      --extra-cxx-flags <f>   Extra CXX flags appended after EXAMPLE/TEST defines.
  -h, --help                  Show this help.

Examples:
  tools/build-example.sh --list
  tools/build-example.sh --list-tests fmac
  tools/build-example.sh --example fmac
  tools/build-example.sh --example EXAMPLE_ADC --test 0 --preset board-debug
  tools/build-example.sh --example ethernet --test TEST_0 --board-name TEST
EOF
}

normalize_example_macro() {
    local input="$1"
    input="${input#EXAMPLE_}"
    input="${input#example_}"
    input="$(printf '%s' "$input" | tr '[:lower:]-' '[:upper:]_')"
    printf 'EXAMPLE_%s' "$input"
}

normalize_test_macro() {
    local input="$1"
    input="${input#TEST_}"
    input="${input#test_}"
    if [[ "$input" =~ ^[0-9]+$ ]]; then
        printf 'TEST_%s' "$input"
        return
    fi
    input="$(printf '%s' "$input" | tr '[:lower:]-' '[:upper:]_')"
    printf 'TEST_%s' "$input"
}

collect_examples() {
    grep -Rho "EXAMPLE_[A-Z0-9_]\+" "${repo_root}/Core/Src/Examples"/*.cpp 2>/dev/null | sort -u
}

find_example_file() {
    local example_macro="$1"
    local file
    for file in "${repo_root}/Core/Src/Examples"/*.cpp; do
        [[ -f "$file" ]] || continue
        if grep -Eq "^[[:space:]]*#(if|ifdef|elif)[[:space:]].*\\b${example_macro}\\b" "$file"; then
            printf '%s\n' "$file"
            return 0
        fi
    done
    return 1
}

collect_tests_for_example() {
    local example_macro="$1"
    local file
    file="$(find_example_file "$example_macro" || true)"
    if [[ -z "$file" ]]; then
        return 1
    fi

    perl -nle 'while(/\b(TEST_[A-Z0-9_]+)\b/g){print $1}' "$file" 2>/dev/null | sort -u || true
}

print_examples_table() {
    local file
    local example_macro
    local tests_csv
    local tests_raw
    local rel_file

    while IFS='|' read -r macro tests file_path; do
        printf "%-40s tests: %s\n" "$macro" "$tests"
        printf "  file: %s\n" "$file_path"
    done < <(
        for file in "${repo_root}/Core/Src/Examples"/*.cpp; do
            [[ -f "$file" ]] || continue
            example_macro="$(grep -Eho "EXAMPLE_[A-Z0-9_]+" "$file" | head -n1 || true)"
            [[ -z "$example_macro" ]] && continue

            tests_raw="$(perl -nle 'while(/\b(TEST_[A-Z0-9_]+)\b/g){print $1}' "$file" 2>/dev/null || true)"
            if [[ -n "$tests_raw" ]]; then
                tests_csv="$(printf '%s\n' "$tests_raw" | sort -u | paste -sd',' -)"
            else
                tests_csv="<none>"
            fi

            rel_file="${file#${repo_root}/}"
            printf "%s|%s|%s\n" "$example_macro" "$tests_csv" "$rel_file"
        done | sort
    )
}

script_dir="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
repo_root="$(CDPATH= cd -- "${script_dir}/.." && pwd)"

list_mode=0
list_tests_target=""
example_name=""
test_macro="TEST_0"
test_explicit=0
preset="board-debug-eth-ksz8041"
board_name=""
extra_cxx_flags=""

while [[ $# -gt 0 ]]; do
    case "$1" in
        -l|--list)
            list_mode=1
            shift
            ;;
        --list-tests)
            list_tests_target="${2:-}"
            shift 2
            ;;
        -e|--example)
            example_name="${2:-}"
            shift 2
            ;;
        -t|--test)
            test_macro="$(normalize_test_macro "${2:-}")"
            test_explicit=1
            shift 2
            ;;
        --no-test)
            test_macro=""
            test_explicit=1
            shift
            ;;
        -p|--preset)
            preset="${2:-}"
            shift 2
            ;;
        -b|--board-name)
            board_name="${2:-}"
            shift 2
            ;;
        --extra-cxx-flags)
            extra_cxx_flags="${2:-}"
            shift 2
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            if [[ -z "$example_name" ]]; then
                example_name="$1"
                shift
            else
                echo "Unknown argument: $1" >&2
                usage
                exit 1
            fi
            ;;
    esac
done

if [[ "$list_mode" -eq 1 ]]; then
    print_examples_table
    exit 0
fi

if [[ -n "$list_tests_target" ]]; then
    example_macro="$(normalize_example_macro "$list_tests_target")"
    if ! find_example_file "$example_macro" >/dev/null; then
        echo "Unknown example macro '${example_macro}'." >&2
        exit 1
    fi
    tests_output="$(collect_tests_for_example "$example_macro" || true)"

    echo "${example_macro}"
    if [[ -z "$tests_output" ]]; then
        echo "  - <none>"
        exit 0
    fi

    while IFS= read -r test_name; do
        [[ -n "$test_name" ]] && echo "  - ${test_name}"
    done <<EOF
${tests_output}
EOF
    exit 0
fi

if [[ -z "$example_name" ]]; then
    echo "Missing required argument: --example" >&2
    usage
    exit 1
fi

example_macro="$(normalize_example_macro "$example_name")"

available_macros=()
while IFS= read -r macro; do
    available_macros+=("$macro")
done < <(collect_examples)

if [[ "${#available_macros[@]}" -gt 0 ]]; then
    found=0
    for macro in "${available_macros[@]}"; do
        if [[ "$macro" == "$example_macro" ]]; then
            found=1
            break
        fi
    done
    if [[ "$found" -ne 1 ]]; then
        echo "Unknown example macro '${example_macro}'." >&2
        echo "Available examples:" >&2
        printf '  - %s\n' "${available_macros[@]}" >&2
        exit 1
    fi
fi

if [[ "$test_explicit" -ne 1 ]]; then
    tests_output="$(collect_tests_for_example "$example_macro" || true)"
    if [[ -z "$tests_output" ]]; then
        test_macro=""
    fi
fi

define_flags="-D${example_macro}"
if [[ -n "$test_macro" ]]; then
    define_flags+=" -D${test_macro}"
fi
if [[ -n "$extra_cxx_flags" ]]; then
    define_flags+=" ${extra_cxx_flags}"
fi

echo "[build-example] repo: ${repo_root}"
echo "[build-example] preset: ${preset}"
echo "[build-example] example: ${example_macro}"
if [[ -n "$test_macro" ]]; then
    echo "[build-example] test: ${test_macro}"
else
    echo "[build-example] test: <none>"
fi

cd "${repo_root}"

configure_cmd=(
    cmake
    --preset "${preset}"
    -DBUILD_EXAMPLES=ON
    "-DCMAKE_CXX_FLAGS=${define_flags}"
)

if [[ -n "$board_name" ]]; then
    configure_cmd+=("-DBOARD_NAME=${board_name}")
fi

"${configure_cmd[@]}"
cmake --build --preset "${preset}"

echo "[build-example] Build completed."
