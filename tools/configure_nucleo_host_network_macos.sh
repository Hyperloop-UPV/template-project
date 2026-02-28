#!/usr/bin/env bash
set -euo pipefail

service_name="USB 10/100/1000 LAN"
iface="en6"
wifi_service="Wi-Fi"
host_ip="192.168.1.9"
subnet_mask="255.255.255.0"
router_ip="0.0.0.0"
board_ip="192.168.1.7"
check_board=0
disable_service=0

usage() {
  cat <<'EOF'
Usage: tools/configure_nucleo_host_network_macos.sh [options]

Safe host-side setup for running Nucleo Ethernet tests while preserving Wi-Fi.

Options:
  --service <name>         Network service name (default: USB 10/100/1000 LAN)
  --iface <name>           Interface device name (default: en6)
  --wifi-service <name>    Wi-Fi service name to keep first (default: Wi-Fi)
  --host-ip <ip>           Host IPv4 for the Nucleo link (default: 192.168.1.9)
  --mask <mask>            Host subnet mask (default: 255.255.255.0)
  --router <ip>            Router for USB service (default: 0.0.0.0)
  --board-ip <ip>          Board IPv4 to preflight (default: 192.168.1.7)
  --check-board            Also run board ping preflight
  --disable-service        Disable the USB service and exit
  -h, --help               Show help
EOF
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --service) service_name="$2"; shift 2 ;;
    --iface) iface="$2"; shift 2 ;;
    --wifi-service) wifi_service="$2"; shift 2 ;;
    --host-ip) host_ip="$2"; shift 2 ;;
    --mask) subnet_mask="$2"; shift 2 ;;
    --router) router_ip="$2"; shift 2 ;;
    --board-ip) board_ip="$2"; shift 2 ;;
    --check-board) check_board=1; shift ;;
    --disable-service) disable_service=1; shift ;;
    -h|--help) usage; exit 0 ;;
    *)
      echo "Unknown option: $1" >&2
      usage
      exit 2
      ;;
  esac
done

if ! command -v networksetup >/dev/null 2>&1; then
  echo "networksetup not found (macOS required)" >&2
  exit 2
fi

if [[ "${disable_service}" -eq 1 ]]; then
  networksetup -setnetworkserviceenabled "${service_name}" off
  echo "Disabled service: ${service_name}"
  exit 0
fi

declare -a all_services=()
while IFS= read -r line; do
  [[ -z "${line}" ]] && continue
  if [[ "${line}" == "An asterisk (*) denotes that a network service is disabled." ]]; then
    continue
  fi
  all_services+=("${line#\*}")
done < <(networksetup -listallnetworkservices)

if [[ ${#all_services[@]} -eq 0 ]]; then
  echo "Could not read network services" >&2
  exit 2
fi

found_service=0
for item in "${all_services[@]}"; do
  if [[ "${item}" == "${service_name}" ]]; then
    found_service=1
    break
  fi
done
if [[ "${found_service}" -ne 1 ]]; then
  echo "Service not found: ${service_name}" >&2
  exit 2
fi

wifi_found=0
for item in "${all_services[@]}"; do
  if [[ "${item}" == "${wifi_service}" ]]; then
    wifi_found=1
    break
  fi
done

if [[ "${wifi_found}" -ne 1 ]]; then
  echo "Wi-Fi service not found as '${wifi_service}', preserving current order for other services." >&2
fi

declare -a reordered=()
if [[ "${wifi_found}" -eq 1 ]]; then
  reordered+=("${wifi_service}")
fi
for item in "${all_services[@]}"; do
  if [[ "${item}" == "${service_name}" ]]; then
    continue
  fi
  if [[ "${wifi_found}" -eq 1 && "${item}" == "${wifi_service}" ]]; then
    continue
  fi
  reordered+=("${item}")
done
reordered+=("${service_name}")

networksetup -ordernetworkservices "${reordered[@]}"
networksetup -setnetworkserviceenabled "${service_name}" on
networksetup -setmanual "${service_name}" "${host_ip}" "${subnet_mask}" "${router_ip}"

echo "CONFIG service=${service_name} iface=${iface} host_ip=${host_ip} board_ip=${board_ip}"
ifconfig "${iface}" | sed -n '1,80p'
echo "---"
route -n get default || true
echo "---"
route -n get "${board_ip}" || true
echo "---"
scutil --nwi | sed -n '1,120p'

if ! ifconfig "${iface}" | rg -Fq "inet ${host_ip} "; then
  echo "Interface ${iface} does not hold expected IPv4 ${host_ip}" >&2
  exit 3
fi

default_if="$(route -n get default 2>/dev/null | awk '/interface:/{print $2; exit}')"
if [[ "${default_if}" == "${iface}" ]]; then
  echo "Default route moved to ${iface}; this may break Wi-Fi. Aborting." >&2
  exit 4
fi

board_route_if="$(route -n get "${board_ip}" 2>/dev/null | awk '/interface:/{print $2; exit}')"
if [[ "${board_route_if}" != "${iface}" ]]; then
  echo "Board route is not using ${iface} (got: ${board_route_if:-none})." >&2
  echo "This is expected on macOS when Wi-Fi owns the 192.168.1.0/24 route." >&2
  echo "Host tools should source-bind to ${host_ip} to reach the Nucleo while preserving Wi-Fi." >&2
fi

internet_probe="$(ping -c 1 -W 1000 1.1.1.1 || true)"
if ! printf '%s\n' "${internet_probe}" | rg -q "bytes from 1.1.1.1"; then
  echo "Internet probe failed after setup; check Wi-Fi/service order." >&2
  exit 6
fi

gateway_ip="$(route -n get default 2>/dev/null | awk '/gateway:/{print $2; exit}')"
if [[ -n "${gateway_ip}" ]]; then
  local_probe="$(nc -vz -w 2 "${gateway_ip}" 80 2>&1 || true)"
  if printf '%s\n' "${local_probe}" | rg -q "No route to host"; then
    cat >&2 <<EOF
Detected local-network block from current process context.
You likely need to grant Local Network permission to the app running this shell
(e.g. Visual Studio Code / Codex extension host) in:
System Settings > Privacy & Security > Local Network
EOF
    exit 7
  fi
fi

if [[ "${check_board}" -eq 1 ]]; then
  board_ping="$(ping -S "${host_ip}" -c 3 -W 1000 "${board_ip}" || true)"
  printf '%s\n' "${board_ping}"
  if ! printf '%s\n' "${board_ping}" | rg -q "bytes from ${board_ip}"; then
    echo "Board ping failed for ${board_ip}" >&2
    exit 8
  fi
fi

echo "HOST_NETWORK_READY"
