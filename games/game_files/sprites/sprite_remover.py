import sys

def remove_const_array_from_c(lines, var_name):
    out = []
    skipping = False
    brace_depth = 0

    for line in lines:
        if not skipping:
            if (
                "const" in line
                and var_name in line
                and "=" in line
            ):
                skipping = True
                brace_depth = line.count("{") - line.count("}")
                if brace_depth <= 0 and ";" in line:
                    skipping = False
                continue
            out.append(line)
        else:
            brace_depth += line.count("{") - line.count("}")
            if brace_depth <= 0 and ";" in line:
                skipping = False

    return out


def remove_const_array_from_h(lines, var_name):
    out = []
    skipping = False

    for line in lines:
        if not skipping:
            if (
                "extern" in line
                and "const" in line
                and var_name in line
                or f"{var_name}_sprite" in line
            ):
                skipping = True
                if ";" in line:
                    skipping = False
                continue
            out.append(line)
        else:
            if ";" in line:
                skipping = False

    return out

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("usage: sprite_remover.py <var_name>")
        sys.exit(1)

    var_name = sys.argv[1]
    with open("sprites.c", "r") as fc:
        lines = fc.readlines()
        new_lines = remove_const_array_from_c(lines,var_name)

    with open("sprites.c", "w") as fc:
        fc.writelines(new_lines)

    with open("sprites.h", "r") as fh:
        lines = fh.readlines()
        new_lines = remove_const_array_from_h(lines,var_name)

    with open("sprites.h", "w") as fh:
        fh.writelines(new_lines)
