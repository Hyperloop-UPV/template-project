#!/usr/bin/env python3
import argparse
import subprocess
import sys
import os

def analyze_elf(elf_path, toolchain_prefix="arm-none-eabi-"):
    if not os.path.isfile(elf_path):
        print(f"Error: ELF file '{elf_path}' does not exist.")
        sys.exit(1)

    nm_command = [f"{toolchain_prefix}nm", "--print-size", "--size-sort", "--radix=d", "-C", elf_path]
    
    try:
        result = subprocess.run(nm_command, capture_output=True, text=True, check=True)
    except subprocess.CalledProcessError as e:
        print(f"Error running nm: {e}")
        print(f"Stderr: {e.stderr}")
        sys.exit(1)
    except FileNotFoundError:
        print(f"Error: Command '{nm_command[0]}' not found. Make sure the toolchain is in your PATH.")
        sys.exit(1)

    lines = result.stdout.splitlines()
    
    functions = []
    globals_vars = []
    
    for line in lines:
        parts = line.strip().split(' ', 3)
        if len(parts) < 4:
            continue
            
        try:
            # Format: Address Size Type Name
            size = int(parts[1])
            type_char = parts[2]
            name = parts[3]
        except ValueError:
            continue
            
        type_upper = type_char.upper()
        
        # Function types: T (Text), W (Weak - often functions)
        if type_upper in ('T', 'W'):
             # Usually W can be function or data, but in C++ context often weak functions. 
             # Let's verify if we want to separate them. 
             # Common practice: treat T/t and W/w as code if they are in .text section.
             # nm doesn't give section, but T usually means text.
             functions.append((size, name, type_char))
        
        # Global variable types: 
        # D: Initialized Data
        # B: BSS (Uninitialized Data)
        # R: Read-only Data
        # 'd', 'b', 'r' are local versions
        elif type_upper in ('D', 'B', 'R'):
            globals_vars.append((size, name, type_char))

    # Sort descending by size
    functions.sort(key=lambda x: x[0], reverse=True)
    globals_vars.sort(key=lambda x: x[0], reverse=True)
    
    print_table("Top 50 Functions by Size (Text)", functions[:50])
    print("\n")
    print_table("Top 50 Global Variables by Size (Data/BSS/RO)", globals_vars[:50])
    print("\n")

    # Analyze Strings
    total_strings_size = analyze_strings(elf_path, toolchain_prefix)

    # Summary
    total_func_size = sum(x[0] for x in functions)
    total_global_size = sum(x[0] for x in globals_vars)
    print("\n")
    print(f"Total analyzed function size: {total_func_size} bytes")
    print(f"Total analyzed global size:   {total_global_size} bytes")
    if total_strings_size:
        print(f"Total analyzed strings size:  {total_strings_size} bytes (Approximate, filtered to .rodata/.data)")

def analyze_strings(elf_path, toolchain_prefix):
    # 1. Get Section info using objdump to filter strings by valid sections (.rodata, .data)
    # This avoids counting debug strings or code as strings.
    objdump_cmd = f"{toolchain_prefix}objdump"
    valid_ranges = []
    
    from shutil import which
    if which(objdump_cmd):
        try:
            # Run objdump -h
            res = subprocess.run([objdump_cmd, "-h", elf_path], capture_output=True, text=True)
            if res.returncode == 0:
                for line in res.stdout.splitlines():
                    parts = line.split()
                    # Idx Name Size VMA LMA File off Algn
                    # 2 .rodata 00005850 08059018 08059018 0005a018 2**3
                    # We look for lines starting with index number, so len > 6 usually.
                    # We care about .rodata, .data, maybe .metadata_pool if it exists?
                    # Let's stick to .rodata and .data for standard literal strings.
                    if len(parts) >= 7 and parts[1] in ['.rodata', '.data']:
                        try:
                            # File off is index 5
                            size = int(parts[2], 16)
                            offset = int(parts[5], 16)
                            valid_ranges.append((offset, offset + size, parts[1]))
                        except ValueError:
                            pass
        except Exception:
            pass

    if not valid_ranges:
        print("Warning: Could not determine section ranges (objdump failed?). Analyzing ALL strings.")
    
    # 2. Run strings command
    # Try to find the strings command
    strings_cmd = f"{toolchain_prefix}strings"
    
    # Check if toolchain strings exists, otherwise use system strings
    if which(strings_cmd) is None:
        strings_cmd = "strings"
        if which(strings_cmd) is None:
            print("Warning: 'strings' command not found. Skipping string analysis.")
            return 0

    # Run strings command (min length 4) with offsets (-t d)
    cmd = [strings_cmd, "-n", "4", "-t", "d", elf_path]
    
    try:
        result = subprocess.run(cmd, capture_output=True, text=True)
        if result.returncode != 0:
            return 0
    except Exception:
        return 0

    # Process strings
    found_strings = []
    total_size = 0
    
    # Parse output: "OFFSET STRING"
    for line in result.stdout.splitlines():
        parts = line.strip().split(' ', 1)
        if len(parts) < 2:
            continue
            
        try:
            offset = int(parts[0])
            s = parts[1]
        except ValueError:
            continue
            
        # Filter: Must be in a valid section (if we found any)
        section = ""
        if valid_ranges:
            in_valid_section = False
            for start, end, name in valid_ranges:
                if start <= offset < end:
                    in_valid_section = True
                    section = name
                    break
            if not in_valid_section:
                continue
        
        # Estimate size: length + 1 (null terminator)
        size = len(s) + 1 
        total_size += size
        
        # Add section name to type for clarity
        type_str = f"str({section})" if section else "str"
        found_strings.append((size, s, type_str))
    
    # Sort
    found_strings.sort(key=lambda x: x[0], reverse=True)
    
    print_table("Top 50 Literal Strings (Filtered by .rodata/.data)", found_strings[:50])
    return total_size

def print_table(title, items):
    print(f"--- {title} ---")
    print(f"{'Size (bytes)':<15} {'Type':<5} {'Name'}")
    print("-" * 120)
    for size, name, type_char in items:
        # Truncate very long names
        display_name = (name[:100] + '...') if len(name) > 100 else name
        print(f"{size:<15} {type_char:<5} {display_name}")
    print("-" * 120)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Analyze ELF binary symbol sizes.")
    parser.add_argument("elf_file", help="Path to the ELF file to analyze")
    parser.add_argument("--toolchain", default="arm-none-eabi-", help="Toolchain prefix (default: arm-none-eabi-)")
    
    args = parser.parse_args()
    analyze_elf(args.elf_file, args.toolchain)
