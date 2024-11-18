import pefile
import sys

if len(sys.argv) < 2:
    print("Error: No file path specified. Usage: python hexdump.py <file_path>")
    sys.exit(1)

file_path = sys.argv[1]

try:
    pe = pefile.PE(file_path)
except FileNotFoundError:
    print(f"Error: File '{file_path}' not found.")
    sys.exit(1)

print(f"Loaded file: {file_path}\n")


def print_summary():
    print("Section Summary:")
    for section in pe.sections:
        print(f"  {section.Name.decode().strip():<10} {section.SizeOfRawData // 1024:>6} kb")
    total = sum(section.SizeOfRawData for section in pe.sections)
    print(f"Total size: {total // 1024} kb\n")


def analyze_section(section_name):
    section_name = section_name if section_name.startswith(".") else f".{section_name}"

    # Locate the section by name
    target_section = None
    for section in pe.sections:
        if section.Name.decode("utf-8").rstrip("\x00") == section_name:
            target_section = section
            break

    if not target_section:
        print(f"Error: Section '{section_name}' not found in the DLL.")
        return

    print(f"\nAnalyzing section: {section_name}")
    print(f"  Section size: {target_section.SizeOfRawData} bytes")
    print(f"  Virtual Address: {hex(target_section.VirtualAddress)}")
    print(f"  Raw Size: {target_section.SizeOfRawData} bytes")
    print(f"  Raw Data Offset: {hex(target_section.PointerToRawData)}")

    # Extract the contents of the section
    raw_data = pe.get_data(target_section.PointerToRawData, target_section.SizeOfRawData)

    # Hex dump with ASCII decoding
    print("\nHex dump of section content:")
    bytes_per_line = 16
    for i in range(0, len(raw_data), bytes_per_line):
        chunk = raw_data[i : i + bytes_per_line]
        hex_part = " ".join(f"{b:02X}" for b in chunk)
        ascii_part = "".join(chr(b) if 32 <= b <= 126 else "." for b in chunk)
        print(f"{i:08X}  {hex_part:<48}  {ascii_part}")

    # Content analysis
    total_zeros = raw_data.count(0)
    total_non_zeros = len(raw_data) - total_zeros
    print("\nContent Analysis:")
    print(f"  Zeros: {total_zeros} bytes")
    print(f"  Non-zero data: {total_non_zeros} bytes\n")


if __name__ == "__main__":
    print_summary()

    print(
        "Enter a section name to analyze (e.g., .data or data), or 'list' to print the summary again, or press Ctrl+C to quit."
    )

    while True:
        try:
            user_input = input("> ").strip()
            if user_input == "list":
                print_summary()
            if user_input == "quit":
                exit()
            elif user_input:
                analyze_section(user_input)
        except KeyboardInterrupt:
            print("\nExiting.")
            break
