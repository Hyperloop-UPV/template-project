#!/usr/bin/env bash
set -euo pipefail

script_dir="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
repo_root="$(CDPATH= cd -- "${script_dir}/.." && pwd)"
stress_script="${repo_root}/tools/run_example_tcpip_stress.sh"

board_ip="192.168.1.7"
host_bind="0.0.0.0"
duration_min=120
interval_sec=2
payload_interval_us=800
min_payload_rx_ratio=0.90
min_bad_detect_ratio=0.80
min_server_burst_ratio=0.95
min_client_burst_ratio=0.95
control_mode="auto"
good_payloads=2000
bad_payloads=400
server_burst=1200
client_burst=1200
udp_count=600
strict_client_stream=0
health_on_fail=1
health_pages=6
health_at_end=0
max_failures=1

usage() {
  cat <<'EOF'
Usage: tools/example_tcpip_soak.sh [options]

Options:
  --board-ip <ip>              Board IP (default: 192.168.1.7)
  --host-bind <ip>             Host bind IP (default: 0.0.0.0)
  --duration-min <n>           Soak duration in minutes (default: 120)
  --interval-sec <n>           Sleep between runs (default: 2)
  --payload-interval-us <n>    Payload pacing (default: 800)
  --control-mode <m>           auto|server|client (default: auto)
  --min-payload-rx-ratio <f>   Min payload RX ratio (default: 0.90)
  --min-bad-detect-ratio <f>   Min bad-checksum detect ratio (default: 0.80)
  --min-server-burst-ratio <f> Min server burst receive ratio (default: 0.95)
  --min-client-burst-ratio <f> Min client burst receive ratio (default: 0.95)
  --good-payloads <n>          Good payloads per run (default: 2000)
  --bad-payloads <n>           Bad payloads per run (default: 400)
  --server-burst <n>           Server burst per run (default: 1200)
  --client-burst <n>           Client burst per run (default: 1200)
  --udp-count <n>              UDP probes per run (default: 600)
  --health-pages <n>           Number of health pages to query (default: 6)
  --health-at-end              Print health pages at the end of each run
  --strict-client-stream       Fail each run on client-stream instability
  --no-health-on-fail          Disable health dump on failures
  --max-failures <n>           Stop soak after this many failures (default: 1)
  -h, --help                   Show help
EOF
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --board-ip) board_ip="$2"; shift 2 ;;
    --host-bind) host_bind="$2"; shift 2 ;;
    --duration-min) duration_min="$2"; shift 2 ;;
    --interval-sec) interval_sec="$2"; shift 2 ;;
    --payload-interval-us) payload_interval_us="$2"; shift 2 ;;
    --control-mode) control_mode="$2"; shift 2 ;;
    --min-payload-rx-ratio) min_payload_rx_ratio="$2"; shift 2 ;;
    --min-bad-detect-ratio) min_bad_detect_ratio="$2"; shift 2 ;;
    --min-server-burst-ratio) min_server_burst_ratio="$2"; shift 2 ;;
    --min-client-burst-ratio) min_client_burst_ratio="$2"; shift 2 ;;
    --good-payloads) good_payloads="$2"; shift 2 ;;
    --bad-payloads) bad_payloads="$2"; shift 2 ;;
    --server-burst) server_burst="$2"; shift 2 ;;
    --client-burst) client_burst="$2"; shift 2 ;;
    --udp-count) udp_count="$2"; shift 2 ;;
    --health-pages) health_pages="$2"; shift 2 ;;
    --health-at-end) health_at_end=1; shift ;;
    --strict-client-stream) strict_client_stream=1; shift ;;
    --no-health-on-fail) health_on_fail=0; shift ;;
    --max-failures) max_failures="$2"; shift 2 ;;
    -h|--help) usage; exit 0 ;;
    *)
      echo "Unknown option: $1" >&2
      usage
      exit 2
      ;;
  esac
done

timestamp="$(date +%Y%m%d_%H%M%S)"
log_dir="${repo_root}/out/soak/${timestamp}"
mkdir -p "${log_dir}"

deadline_epoch=$(( $(date +%s) + duration_min * 60 ))
run_idx=0
pass_count=0
fail_count=0

echo "SOAK_START duration_min=${duration_min} interval_sec=${interval_sec} board_ip=${board_ip} log_dir=${log_dir}"

while [[ $(date +%s) -lt ${deadline_epoch} ]]; do
  run_idx=$((run_idx + 1))
  log_file="${log_dir}/soak_run_${run_idx}.log"
  cmd=(
    "${stress_script}"
    --board-ip "${board_ip}"
    --host-bind "${host_bind}"
    --control-mode "${control_mode}"
    --payload-interval-us "${payload_interval_us}"
    --min-payload-rx-ratio "${min_payload_rx_ratio}"
    --min-bad-detect-ratio "${min_bad_detect_ratio}"
    --min-server-burst-ratio "${min_server_burst_ratio}"
    --min-client-burst-ratio "${min_client_burst_ratio}"
    --good-payloads "${good_payloads}"
    --bad-payloads "${bad_payloads}"
    --server-burst "${server_burst}"
    --client-burst "${client_burst}"
    --udp-count "${udp_count}"
    --health-pages "${health_pages}"
    --reset-health
  )
  if [[ "${strict_client_stream}" -eq 1 ]]; then
    cmd+=(--strict-client-stream)
  fi
  if [[ "${health_on_fail}" -eq 0 ]]; then
    cmd+=(--no-health-on-fail)
  fi
  if [[ "${health_at_end}" -eq 1 ]]; then
    cmd+=(--health-at-end)
  fi

  echo "[SOAK] run=${run_idx} -> ${log_file}"
  if "${cmd[@]}" >"${log_file}" 2>&1; then
    pass_count=$((pass_count + 1))
    overall="$(rg -n "^Overall:" "${log_file}" | tail -n1 | cut -d: -f2- || true)"
    echo "[SOAK] run=${run_idx} PASS ${overall}"
  else
    fail_count=$((fail_count + 1))
    overall="$(rg -n "^Overall:" "${log_file}" | tail -n1 | cut -d: -f2- || true)"
    echo "[SOAK] run=${run_idx} FAIL ${overall}"
    if [[ "${fail_count}" -ge "${max_failures}" ]]; then
      echo "[SOAK] stopping early: fail_count=${fail_count} reached max_failures=${max_failures}"
      break
    fi
  fi

  sleep "${interval_sec}"
done

echo
echo "SOAK_SUMMARY runs=${run_idx} pass=${pass_count} fail=${fail_count} log_dir=${log_dir}"
if [[ "${fail_count}" -eq 0 ]]; then
  echo "SOAK_RESULT PASS"
  exit 0
fi

echo "SOAK_RESULT FAIL"
exit 1
