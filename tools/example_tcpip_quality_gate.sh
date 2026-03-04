#!/usr/bin/env bash
set -euo pipefail

script_dir="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
repo_root="$(CDPATH= cd -- "${script_dir}/.." && pwd)"
stress_script="${repo_root}/tools/run_example_tcpip_stress.sh"

board_ip="192.168.1.7"
host_bind="0.0.0.0"
base_runs=20
aggr_runs=5
base_good=1200
base_bad=200
base_server=800
base_client=800
base_udp=300
aggr_good=5000
aggr_bad=1000
aggr_server=4000
aggr_client=4000
aggr_udp=2000
payload_interval_us=800
min_payload_rx_ratio=0.90
min_bad_detect_ratio=0.80
min_server_burst_ratio=0.95
min_client_burst_ratio=0.95
control_mode="auto"
strict_client_stream=1
health_on_fail=1
health_pages=6
health_at_end=0
stop_on_first_fail=0
inter_run_sleep_s="1.5"
preflight_ping_retries=8

usage() {
  cat <<'EOF'
Usage: tools/example_tcpip_quality_gate.sh [options]

Options:
  --board-ip <ip>              Board IP (default: 192.168.1.7)
  --host-bind <ip>             Host bind IP (default: 0.0.0.0)
  --base-runs <n>              Number of strict base runs (default: 20)
  --aggr-runs <n>              Number of strict aggressive runs (default: 5)
  --payload-interval-us <n>    Payload pacing (default: 800)
  --base-good <n>              Base good payloads (default: 1200)
  --base-bad <n>               Base bad payloads (default: 200)
  --base-server <n>            Base server burst (default: 800)
  --base-client <n>            Base client burst (default: 800)
  --base-udp <n>               Base udp count (default: 300)
  --aggr-good <n>              Aggressive good payloads (default: 5000)
  --aggr-bad <n>               Aggressive bad payloads (default: 1000)
  --aggr-server <n>            Aggressive server burst (default: 4000)
  --aggr-client <n>            Aggressive client burst (default: 4000)
  --aggr-udp <n>               Aggressive udp count (default: 2000)
  --control-mode <m>           auto|server|client (default: auto)
  --min-payload-rx-ratio <f>   Min payload RX ratio (default: 0.90)
  --min-bad-detect-ratio <f>   Min bad-checksum detect ratio (default: 0.80)
  --min-server-burst-ratio <f> Min server burst receive ratio (default: 0.95)
  --min-client-burst-ratio <f> Min client burst receive ratio (default: 0.95)
  --health-pages <n>           Number of health pages per run (default: 6)
  --health-at-end              Print health pages at the end of each run
  --no-strict-client-stream    Do not enforce strict tcp_client_stream
  --no-health-on-fail          Disable health dumps on failed runs
  --stop-on-first-fail         Stop quality gate immediately on first failed run
  --inter-run-sleep <s>        Sleep time between runs in seconds (default: 1.5)
  --preflight-ping-retries <n> Number of ping retries before each run (default: 8)
  -h, --help                   Show help
EOF
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --board-ip) board_ip="$2"; shift 2 ;;
    --host-bind) host_bind="$2"; shift 2 ;;
    --base-runs) base_runs="$2"; shift 2 ;;
    --aggr-runs) aggr_runs="$2"; shift 2 ;;
    --payload-interval-us) payload_interval_us="$2"; shift 2 ;;
    --base-good) base_good="$2"; shift 2 ;;
    --base-bad) base_bad="$2"; shift 2 ;;
    --base-server) base_server="$2"; shift 2 ;;
    --base-client) base_client="$2"; shift 2 ;;
    --base-udp) base_udp="$2"; shift 2 ;;
    --aggr-good) aggr_good="$2"; shift 2 ;;
    --aggr-bad) aggr_bad="$2"; shift 2 ;;
    --aggr-server) aggr_server="$2"; shift 2 ;;
    --aggr-client) aggr_client="$2"; shift 2 ;;
    --aggr-udp) aggr_udp="$2"; shift 2 ;;
    --control-mode) control_mode="$2"; shift 2 ;;
    --min-payload-rx-ratio) min_payload_rx_ratio="$2"; shift 2 ;;
    --min-bad-detect-ratio) min_bad_detect_ratio="$2"; shift 2 ;;
    --min-server-burst-ratio) min_server_burst_ratio="$2"; shift 2 ;;
    --min-client-burst-ratio) min_client_burst_ratio="$2"; shift 2 ;;
    --health-pages) health_pages="$2"; shift 2 ;;
    --health-at-end) health_at_end=1; shift ;;
    --no-strict-client-stream) strict_client_stream=0; shift ;;
    --no-health-on-fail) health_on_fail=0; shift ;;
    --stop-on-first-fail) stop_on_first_fail=1; shift ;;
    --inter-run-sleep) inter_run_sleep_s="$2"; shift 2 ;;
    --preflight-ping-retries) preflight_ping_retries="$2"; shift 2 ;;
    -h|--help) usage; exit 0 ;;
    *)
      echo "Unknown option: $1" >&2
      usage
      exit 2
      ;;
  esac
done

timestamp="$(date +%Y%m%d_%H%M%S)"
log_dir="${repo_root}/out/quality-gate/${timestamp}"
mkdir -p "${log_dir}"

run_case() {
  local suite="$1"
  local run_idx="$2"
  shift 2
  local -a cmd=("$@")

  local log_file="${log_dir}/${suite}_run_${run_idx}.log"
  echo "[$suite] run ${run_idx} -> ${log_file}"
  if "${cmd[@]}" >"${log_file}" 2>&1; then
    local overall
    overall="$(rg -n "^Overall:" "${log_file}" | tail -n1 | cut -d: -f2- || true)"
    echo "[$suite] run ${run_idx} PASS ${overall}"
    return 0
  fi

  local overall
  overall="$(rg -n "^Overall:" "${log_file}" | tail -n1 | cut -d: -f2- || true)"
  echo "[$suite] run ${run_idx} FAIL ${overall}"
  return 1
}

wait_board_reachable() {
  local retries="$1"
  local i
  for ((i=1; i<=retries; i++)); do
    if ping -c 1 -W 1000 "${board_ip}" >/dev/null 2>&1; then
      return 0
    fi
    sleep 0.5
  done
  return 1
}

base_failures=0
aggr_failures=0

declare -a common_args
common_args=(
  --board-ip "${board_ip}"
  --host-bind "${host_bind}"
  --control-mode "${control_mode}"
  --payload-interval-us "${payload_interval_us}"
  --min-payload-rx-ratio "${min_payload_rx_ratio}"
  --min-bad-detect-ratio "${min_bad_detect_ratio}"
  --min-server-burst-ratio "${min_server_burst_ratio}"
  --min-client-burst-ratio "${min_client_burst_ratio}"
  --health-pages "${health_pages}"
  --reset-health
)
if [[ "${strict_client_stream}" -eq 1 ]]; then
  common_args+=(--strict-client-stream)
fi
if [[ "${health_on_fail}" -eq 0 ]]; then
  common_args+=(--no-health-on-fail)
fi
if [[ "${health_at_end}" -eq 1 ]]; then
  common_args+=(--health-at-end)
fi

for ((i=1; i<=base_runs; i++)); do
  if ! wait_board_reachable "${preflight_ping_retries}"; then
    echo "[base] run ${i} FAIL preflight_ping board_ip=${board_ip}"
    base_failures=$((base_failures + 1))
    if [[ "${stop_on_first_fail}" -eq 1 ]]; then
      break
    fi
    sleep "${inter_run_sleep_s}"
    continue
  fi

  if ! run_case "base" "${i}" \
    "${stress_script}" \
    "${common_args[@]}" \
    --good-payloads "${base_good}" \
    --bad-payloads "${base_bad}" \
    --server-burst "${base_server}" \
    --client-burst "${base_client}" \
    --udp-count "${base_udp}"; then
    base_failures=$((base_failures + 1))
    if [[ "${stop_on_first_fail}" -eq 1 ]]; then
      break
    fi
  fi
  sleep "${inter_run_sleep_s}"
done

if [[ "${stop_on_first_fail}" -eq 0 || "${base_failures}" -eq 0 ]]; then
  for ((i=1; i<=aggr_runs; i++)); do
    if ! wait_board_reachable "${preflight_ping_retries}"; then
      echo "[aggressive] run ${i} FAIL preflight_ping board_ip=${board_ip}"
      aggr_failures=$((aggr_failures + 1))
      if [[ "${stop_on_first_fail}" -eq 1 ]]; then
        break
      fi
      sleep "${inter_run_sleep_s}"
      continue
    fi

    if ! run_case "aggressive" "${i}" \
      "${stress_script}" \
      "${common_args[@]}" \
      --good-payloads "${aggr_good}" \
      --bad-payloads "${aggr_bad}" \
      --server-burst "${aggr_server}" \
      --client-burst "${aggr_client}" \
      --udp-count "${aggr_udp}"; then
      aggr_failures=$((aggr_failures + 1))
      if [[ "${stop_on_first_fail}" -eq 1 ]]; then
        break
      fi
    fi
    sleep "${inter_run_sleep_s}"
  done
fi

echo
echo "QUALITY_GATE_SUMMARY base_runs=${base_runs} base_failures=${base_failures} aggr_runs=${aggr_runs} aggr_failures=${aggr_failures} log_dir=${log_dir}"
if [[ "${base_failures}" -eq 0 && "${aggr_failures}" -eq 0 ]]; then
  echo "QUALITY_GATE_RESULT PASS"
  exit 0
fi

echo "QUALITY_GATE_RESULT FAIL"
exit 1
