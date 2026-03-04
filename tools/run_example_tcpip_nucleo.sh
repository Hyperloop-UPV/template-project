#!/usr/bin/env bash
set -euo pipefail

script_dir="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
repo_root="$(CDPATH= cd -- "${script_dir}/.." && pwd)"

iface="en6"
board_ip="192.168.1.7"
host_ip=""
host_bind="0.0.0.0"
preset="nucleo-debug-eth"
jobs=8
flash_method="stm32prog"
base_runs=1
aggr_runs=0
control_mode="auto"
strict_client_stream=1
health_on_fail=1
health_at_end=1
skip_build=0
skip_flash=0
skip_ping=0
skip_tests=0
skip_localnet_check=0

usage() {
  cat <<'EOF'
Usage: tools/run_example_tcpip_nucleo.sh [options]

End-to-end flow:
1) Detect host IP from interface (or use explicit override)
2) Build ExampleTCPIP with TCPIP_TEST_HOST_IP=<host_ip>
3) Flash Nucleo
4) Verify network reachability (ping/arp)
5) Run quality gate matrix

Options:
  --iface <name>               Host interface for host IP detection (default: en6)
  --board-ip <ip>              Board IP (default: 192.168.1.7)
  --host-ip <ip>               Explicit host IPv4 (default: auto-detect on iface)
  --host-bind <ip>             Host bind IP for tests (default: 0.0.0.0)
  --preset <name>              CMake preset (default: nucleo-debug-eth)
  --jobs <n>                   Build parallelism (default: 8)
  --flash-method <m>           stm32prog|openocd (default: stm32prog)
  --base-runs <n>              Base quality-gate runs (default: 1)
  --aggr-runs <n>              Aggressive quality-gate runs (default: 0)
  --control-mode <m>           auto|server|client (default: auto)
  --no-strict-client-stream    Disable strict tcp_client_stream checks
  --no-health-on-fail          Disable health dump on failed tests
  --no-health-at-end           Disable end-of-run health dump
  --skip-build                 Skip cmake configure/build
  --skip-flash                 Skip flashing step
  --skip-ping                  Skip ping/arp network check
  --skip-localnet-check        Skip host local-network diagnostics
  --skip-tests                 Skip quality-gate execution
  -h, --help                   Show help
EOF
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --iface) iface="$2"; shift 2 ;;
    --board-ip) board_ip="$2"; shift 2 ;;
    --host-ip) host_ip="$2"; shift 2 ;;
    --host-bind) host_bind="$2"; shift 2 ;;
    --preset) preset="$2"; shift 2 ;;
    --jobs) jobs="$2"; shift 2 ;;
    --flash-method) flash_method="$2"; shift 2 ;;
    --base-runs) base_runs="$2"; shift 2 ;;
    --aggr-runs) aggr_runs="$2"; shift 2 ;;
    --control-mode) control_mode="$2"; shift 2 ;;
    --no-strict-client-stream) strict_client_stream=0; shift ;;
    --no-health-on-fail) health_on_fail=0; shift ;;
    --no-health-at-end) health_at_end=0; shift ;;
    --skip-build) skip_build=1; shift ;;
    --skip-flash) skip_flash=1; shift ;;
    --skip-ping) skip_ping=1; shift ;;
    --skip-localnet-check) skip_localnet_check=1; shift ;;
    --skip-tests) skip_tests=1; shift ;;
    -h|--help) usage; exit 0 ;;
    *)
      echo "Unknown option: $1" >&2
      usage
      exit 2
      ;;
  esac
done

if [[ -z "${host_ip}" ]]; then
  board_prefix="${board_ip%.*}."
  while IFS= read -r candidate; do
    if [[ "${candidate}" == "${board_prefix}"* ]]; then
      host_ip="${candidate}"
      break
    fi
  done < <(ifconfig "${iface}" 2>/dev/null | awk '/inet /{print $2}')
fi

if [[ -z "${host_ip}" ]]; then
  host_ip="$(ipconfig getifaddr "${iface}" || true)"
fi

if [[ -z "${host_ip}" ]]; then
  echo "Could not determine host IPv4 on interface ${iface}" >&2
  echo "Tip: configure the host-side board-link interface manually or pass --host-ip." >&2
  exit 2
fi

echo "RUN_CONTEXT iface=${iface} host_ip=${host_ip} board_ip=${board_ip} preset=${preset}"

effective_host_bind="${host_bind}"
if [[ "${effective_host_bind}" == "0.0.0.0" ]]; then
  effective_host_bind="${host_ip}"
fi

if ! ifconfig "${iface}" 2>/dev/null | rg -Fq "inet ${host_ip} "; then
  echo "Interface ${iface} does not currently hold host_ip ${host_ip}" >&2
  exit 2
fi

board_route_output="$(route -n get "${board_ip}" 2>/dev/null || true)"
board_route_iface="$(printf '%s\n' "${board_route_output}" | awk '/interface:/{print $2; exit}')"
if [[ "${board_route_iface}" != "${iface}" ]]; then
  echo "${board_route_output}"
  echo "Board route does not use ${iface} (got: ${board_route_iface:-none})." >&2
  echo "Proceeding because source-binding to ${host_ip} can still reach the board even if the OS route points elsewhere." >&2
fi

if [[ "${skip_localnet_check}" -eq 0 ]]; then
  default_gateway="$(route -n get default 2>/dev/null | awk '/gateway:/{print $2; exit}')"
  internet_probe="$(ping -c 1 -W 1000 1.1.1.1 || true)"
  if printf '%s\n' "${internet_probe}" | rg -q "bytes from 1.1.1.1" && [[ -n "${default_gateway}" ]]; then
    local_probe="$(nc -vz -w 2 "${default_gateway}" 80 2>&1 || true)"
    if printf '%s\n' "${local_probe}" | rg -q "No route to host"; then
      cat >&2 <<'EOF'
Host can reach internet but cannot open local-network routes from this process context.
The current app or shell process may be blocked from using local-network routes,
or the OS may require an explicit permission for local-network access.
EOF
      exit 2
    fi
  fi
fi

cd "${repo_root}"

if [[ "${skip_build}" -eq 0 ]]; then
  cmake --preset "${preset}" \
    -DBUILD_EXAMPLES=ON \
    -DCMAKE_CXX_FLAGS="-DEXAMPLE_TCPIP -DTCPIP_TEST_HOST_IP=${host_ip} -DTCPIP_TEST_BOARD_IP=${board_ip}"
  cmake --build --preset "${preset}" -j"${jobs}"
fi

if [[ "${skip_flash}" -eq 0 ]]; then
  case "${flash_method}" in
    stm32prog)
      STM32_Programmer_CLI -c port=SWD mode=UR -w out/build/latest.elf -v -rst
      ;;
    openocd)
      if ! openocd -f .vscode/stlink.cfg -f .vscode/stm32h7x.cfg \
        -c "program out/build/latest.elf verify reset exit"; then
        echo "OpenOCD verify failed, retrying flash without verify (RAM sections may cause false mismatch)." >&2
        openocd -f .vscode/stlink.cfg -f .vscode/stm32h7x.cfg \
          -c "program out/build/latest.elf reset exit"
      fi
      ;;
    *)
      echo "Unsupported flash method: ${flash_method}" >&2
      exit 2
      ;;
  esac
fi

if [[ "${skip_ping}" -eq 0 ]]; then
  ping_output="$(ping -S "${host_ip}" -c 5 "${board_ip}" || true)"
  echo "${ping_output}"
  echo "---"
  arp -a | rg -F "${board_ip}" || true
  if ! printf '%s\n' "${ping_output}" | rg -Fq "bytes from ${board_ip}"; then
    if printf '%s\n' "${ping_output}" | rg -Fq "No route to host"; then
      echo "ICMP failed with 'No route to host'." >&2
      echo "Route diagnostics:" >&2
      route -n get "${board_ip}" >&2 || true
      echo "NWI diagnostics:" >&2
      scutil --nwi >&2 || true
    fi
    echo "Board is not reachable over ICMP after flash (board_ip=${board_ip})" >&2
    exit 3
  fi
fi

if [[ "${skip_tests}" -eq 0 ]]; then
  cmd=(
    ./tools/example_tcpip_quality_gate.sh
    --board-ip "${board_ip}"
    --host-bind "${effective_host_bind}"
    --base-runs "${base_runs}"
    --aggr-runs "${aggr_runs}"
    --control-mode "${control_mode}"
  )
  if [[ "${strict_client_stream}" -eq 0 ]]; then
    cmd+=(--no-strict-client-stream)
  fi
  if [[ "${health_on_fail}" -eq 0 ]]; then
    cmd+=(--no-health-on-fail)
  fi
  if [[ "${health_at_end}" -eq 1 ]]; then
    cmd+=(--health-at-end)
  fi

  "${cmd[@]}"
fi
