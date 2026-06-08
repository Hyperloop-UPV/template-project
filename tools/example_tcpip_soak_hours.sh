#!/usr/bin/env bash
set -euo pipefail
export LC_ALL=C

script_dir="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
repo_root="$(CDPATH= cd -- "${script_dir}/.." && pwd)"
soak_script="${repo_root}/tools/example_tcpip_soak.sh"

board_ip="192.168.1.7"
host_bind="0.0.0.0"
hours=8
interval_sec=1
payload_interval_us=800
control_mode="auto"
strict_client_stream=1
max_failures=999999
min_pass_ratio="0.90"
baseline_pass_ratio=""

good_payloads=2500
bad_payloads=500
server_burst=1500
client_burst=1500
udp_count=800
health_pages=6
health_on_fail=1
health_at_end=0

usage() {
  cat <<'EOF'
Usage: tools/example_tcpip_soak_hours.sh [options]

Long soak wrapper intended for multi-hour / overnight runs.
Runs tools/example_tcpip_soak.sh and prints pass ratio summary.

Options:
  --board-ip <ip>              Board IP (default: 192.168.1.7)
  --host-bind <ip>             Host bind IP (default: 0.0.0.0)
  --hours <n>                  Duration in hours (default: 8)
  --interval-sec <n>           Sleep between runs (default: 1)
  --payload-interval-us <n>    Payload pacing (default: 800)
  --control-mode <m>           auto|server|client (default: auto)
  --no-strict-client-stream    Disable strict client-stream check
  --max-failures <n>           Stop soak after N failures (default: 999999)
  --min-pass-ratio <f>         Required pass ratio [0..1] (default: 0.90)
  --baseline-pass-ratio <f>    Optional baseline ratio to compare against
  --good-payloads <n>          Good payloads per run (default: 2500)
  --bad-payloads <n>           Bad payloads per run (default: 500)
  --server-burst <n>           Server burst per run (default: 1500)
  --client-burst <n>           Client burst per run (default: 1500)
  --udp-count <n>              UDP probes per run (default: 800)
  --health-pages <n>           Number of health pages (default: 6)
  --no-health-on-fail          Disable health dump on failures
  --health-at-end              Print health pages at end of each run
  -h, --help                   Show help
EOF
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --board-ip) board_ip="$2"; shift 2 ;;
    --host-bind) host_bind="$2"; shift 2 ;;
    --hours) hours="$2"; shift 2 ;;
    --interval-sec) interval_sec="$2"; shift 2 ;;
    --payload-interval-us) payload_interval_us="$2"; shift 2 ;;
    --control-mode) control_mode="$2"; shift 2 ;;
    --no-strict-client-stream) strict_client_stream=0; shift ;;
    --max-failures) max_failures="$2"; shift 2 ;;
    --min-pass-ratio) min_pass_ratio="$2"; shift 2 ;;
    --baseline-pass-ratio) baseline_pass_ratio="$2"; shift 2 ;;
    --good-payloads) good_payloads="$2"; shift 2 ;;
    --bad-payloads) bad_payloads="$2"; shift 2 ;;
    --server-burst) server_burst="$2"; shift 2 ;;
    --client-burst) client_burst="$2"; shift 2 ;;
    --udp-count) udp_count="$2"; shift 2 ;;
    --health-pages) health_pages="$2"; shift 2 ;;
    --no-health-on-fail) health_on_fail=0; shift ;;
    --health-at-end) health_at_end=1; shift ;;
    -h|--help) usage; exit 0 ;;
    *)
      echo "Unknown option: $1" >&2
      usage
      exit 2
      ;;
  esac
done

if ! awk -v x="${hours}" 'BEGIN{exit !(x > 0)}'; then
  echo "--hours must be > 0" >&2
  exit 2
fi

for ratio_name in min_pass_ratio baseline_pass_ratio; do
  ratio_value="${!ratio_name}"
  if [[ -z "${ratio_value}" ]]; then
    continue
  fi
  if ! awk -v x="${ratio_value}" 'BEGIN{exit !(x >= 0 && x <= 1)}'; then
    echo "--${ratio_name//_/-} must be in [0,1]" >&2
    exit 2
  fi
done

duration_min="$(awk -v h="${hours}" 'BEGIN{m=int(h*60 + 0.9999); if (m < 1) m=1; print m}')"
timestamp="$(date +%Y%m%d_%H%M%S)"
report_dir="${repo_root}/out/soak-hours"
mkdir -p "${report_dir}"
session_log="${report_dir}/${timestamp}.log"

if ! ping -c 2 -W 1000 "${board_ip}" >/dev/null 2>&1; then
  echo "Board is not reachable before soak start: ${board_ip}" >&2
  exit 3
fi

cmd=(
  "${soak_script}"
  --board-ip "${board_ip}"
  --host-bind "${host_bind}"
  --duration-min "${duration_min}"
  --interval-sec "${interval_sec}"
  --payload-interval-us "${payload_interval_us}"
  --control-mode "${control_mode}"
  --good-payloads "${good_payloads}"
  --bad-payloads "${bad_payloads}"
  --server-burst "${server_burst}"
  --client-burst "${client_burst}"
  --udp-count "${udp_count}"
  --health-pages "${health_pages}"
  --max-failures "${max_failures}"
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

echo "SOAK_HOURS_START hours=${hours} duration_min=${duration_min} board_ip=${board_ip} session_log=${session_log}"
set +e
"${cmd[@]}" | tee "${session_log}"
soak_exit=${PIPESTATUS[0]}
set -e

summary_line="$(rg -n "^SOAK_SUMMARY" "${session_log}" | tail -n1 | cut -d: -f2- || true)"
if [[ -z "${summary_line}" ]]; then
  echo "SOAK_HOURS_RESULT FAIL reason=no_summary session_log=${session_log}"
  exit "${soak_exit}"
fi

extract_field() {
  local key="$1"
  local line="$2"
  awk -v k="${key}" '
    {
      for (i = 1; i <= NF; i++) {
        split($i, kv, "=")
        if (kv[1] == k) {
          print substr($i, length(k) + 2)
          exit
        }
      }
    }
  ' <<<"${line}"
}

runs="$(extract_field "runs" "${summary_line}")"
pass_count="$(extract_field "pass" "${summary_line}")"
fail_count="$(extract_field "fail" "${summary_line}")"
log_dir="$(extract_field "log_dir" "${summary_line}")"

if [[ -z "${runs}" || -z "${pass_count}" || -z "${fail_count}" ]]; then
  echo "SOAK_HOURS_RESULT FAIL reason=invalid_summary session_log=${session_log}"
  exit 1
fi

pass_ratio="$(awk -v p="${pass_count}" -v r="${runs}" 'BEGIN{ if (r==0) printf "0.0000"; else printf "%.4f", p/r }')"

echo "SOAK_HOURS_SUMMARY runs=${runs} pass=${pass_count} fail=${fail_count} pass_ratio=${pass_ratio} required_min_ratio=${min_pass_ratio} log_dir=${log_dir} session_log=${session_log}"

if [[ -n "${log_dir}" && -d "${log_dir}" ]]; then
  fail_names="$(rg -n "^Overall: FAIL .*-> " "${log_dir}"/soak_run_*.log 2>/dev/null | sed 's/^.*-> //' || true)"
  if [[ -n "${fail_names}" ]]; then
    while IFS= read -r grouped_line; do
      count="$(printf '%s\n' "${grouped_line}" | awk '{print $1}')"
      name="$(printf '%s\n' "${grouped_line}" | sed 's/^[[:space:]]*[0-9][0-9]*[[:space:]]*//')"
      echo "SOAK_HOURS_FAIL_BREAKDOWN name=${name} count=${count}"
    done < <(printf '%s\n' "${fail_names}" | sort | uniq -c)
  fi
fi

if [[ -n "${baseline_pass_ratio}" ]]; then
  ratio_delta="$(awk -v current="${pass_ratio}" -v baseline="${baseline_pass_ratio}" 'BEGIN{printf "%.4f", current-baseline}')"
  echo "SOAK_HOURS_BASELINE baseline=${baseline_pass_ratio} current=${pass_ratio} delta=${ratio_delta}"
fi

if ! awk -v current="${pass_ratio}" -v required="${min_pass_ratio}" 'BEGIN{exit !(current >= required)}'; then
  echo "SOAK_HOURS_RESULT FAIL reason=pass_ratio_below_threshold"
  exit 1
fi

echo "SOAK_HOURS_RESULT PASS"
exit 0
