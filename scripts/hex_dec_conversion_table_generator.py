lines = []

lines.append(["Hex", "Binary", "Dec", "u-Dec"])

for i in range(0, 256):
    lines.append([
        f"{i:X}", # hex
        f"{i:b}", # bin
        f"{i if i < 128 else i - 256}", # signed dec
        f"{i:d}" # unsigned dec
        ])
    
for line in lines:
    print(line)