#!/usr/bin/env python3
import argparse
import os
import subprocess
from datetime import datetime
from pathlib import Path


DUMP_FILE = Path("dump.bin")


def read_metadata(address: str) -> bytes:
    result = subprocess.run(
        [
            "STM32_Programmer_CLI",
            "-c",
            "port=swd",
            "mode=ur",
            "Freq=4000",
            "-u",
            address,
            "0xFF",
            str(DUMP_FILE),
        ],
        check=False,
    )
    if result.returncode != 0:
        if DUMP_FILE.exists():
            DUMP_FILE.unlink()
        raise RuntimeError(
            "Error running STM32_Programmer_CLI. Ensure board power, cable and ST-LINK availability."
        )
    return DUMP_FILE.read_bytes()


def validate_checksum(binary_raw: bytes, address: str, checksum_length: int) -> None:
    if len(binary_raw) < checksum_length:
        raise RuntimeError(f"Retrieved data is too short (address: {address})")
    if any(byte != ord("*") for byte in binary_raw[:checksum_length]):
        raise RuntimeError(
            f"Retrieved binary did not pass checksum test. Maybe metadata is not at {address}"
        )


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Retrieve binary metadata from flash given an address"
    )
    parser.add_argument(
        "--address",
        type=str,
        default="0x080DFD00",
        help="Memory address to read from",
    )
    args = parser.parse_args()

    try:
        binary_raw = read_metadata(args.address)
    except Exception as exc:
        print(exc)
        raise SystemExit(1)

    checksum_length = 16
    iso_time_offset = checksum_length
    iso_time_length = 15
    padding_length = 1
    stlib_commit_offset = iso_time_offset + iso_time_length + padding_length
    stlib_commit_length = 8
    adj_commit_offset = stlib_commit_offset + stlib_commit_length
    adj_commit_length = 8
    board_commit_offset = adj_commit_offset + adj_commit_length
    board_commit_length = 8
    custom_variables_offset = board_commit_offset + board_commit_length

    try:
        validate_checksum(binary_raw, args.address, checksum_length)
    except Exception as exc:
        print(exc)
        raise SystemExit(1)

    print("Found binary metadata!")

    iso_time = binary_raw[iso_time_offset : iso_time_offset + iso_time_length].decode(
        errors="ignore"
    )
    stlib_commit = binary_raw[
        stlib_commit_offset : stlib_commit_offset + stlib_commit_length
    ].decode(errors="ignore")
    adj_commit = binary_raw[
        adj_commit_offset : adj_commit_offset + adj_commit_length
    ].decode(errors="ignore")
    board_commit = binary_raw[
        board_commit_offset : board_commit_offset + board_commit_length
    ].decode(errors="ignore")

    dt = datetime.strptime(iso_time, "%Y%m%dT%H%M%S")
    readable_time = dt.strftime("%d %B %Y, %H:%M:%S")

    print(f"Code was compiled at: {readable_time}")
    print(f"STLIB commit {stlib_commit}")
    print(f"ADJ commit {adj_commit}")
    print(f"Board commit {board_commit}")

    custom_payload = binary_raw[custom_variables_offset:255].decode(errors="ignore")
    print(custom_payload.rstrip("\x00"))

    # Keep compatibility with previous behavior: cleanup temporary dump.
    if DUMP_FILE.exists():
        os.remove(DUMP_FILE)


if __name__ == "__main__":
    main()
