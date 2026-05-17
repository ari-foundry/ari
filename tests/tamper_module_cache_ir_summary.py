#!/usr/bin/env python3

import sys


FNV_OFFSET = 14695981039346656037
FNV_PRIME = 1099511628211
MASK64 = (1 << 64) - 1


def fnv1a64(text):
    value = FNV_OFFSET
    for byte in text.encode("utf-8"):
        value ^= byte
        value = (value * FNV_PRIME) & MASK64
    return f"{value:016x}"


def main():
    if len(sys.argv) != 6:
        print("usage: tamper_module_cache_ir_summary.py INPUT OUTPUT MODULE OLD NEW", file=sys.stderr)
        return 2

    input_path, output_path, module_name, old, new = sys.argv[1:]
    replaced = False
    output_lines = []

    with open(input_path, "r", encoding="utf-8") as handle:
        for line in handle.read().splitlines():
            fields = line.split("\t")
            if (
                not replaced
                and len(fields) == 8
                and fields[0] == "ir-summary"
                and fields[1] == module_name
            ):
                payload = fields[6]
                if old not in payload:
                    print(
                        f"IR summary for module {module_name!r} did not contain {old!r}",
                        file=sys.stderr,
                    )
                    return 1
                fields[6] = payload.replace(old, new, 1)
                fields[5] = fnv1a64(fields[6])
                line = "\t".join(fields)
                replaced = True
            output_lines.append(line)

    if not replaced:
        print(f"no ir-summary record found for module {module_name!r}", file=sys.stderr)
        return 1

    with open(output_path, "w", encoding="utf-8") as handle:
        handle.write("\n".join(output_lines))
        handle.write("\n")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
