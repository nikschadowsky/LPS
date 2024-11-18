ROW_WIDTH = 8

lines = []

# lines.append(["Hex", "Binary", "Dec", "u-Dec"])

for i in range(0, 256):
    lines.append([
        f"{i:X}", # hex
        f"{i:b}", # bin
        f"{i if i < 128 else i - 256}", # signed dec
        f"{i:d}" # unsigned dec
        ])
    
print(f'{"Hex".rjust(8)} | {"Binary".rjust(8)} | {"Dec".rjust(8)} | {"u-Dec".rjust(8)}')
print('-' * 41)

for line in lines:
    print(f'{line[0].rjust(8)} | {line[1].rjust(8)} | {line[2].rjust(8)} | {line[3].rjust(8)}')