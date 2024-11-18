ROW_WIDTH = 8

file = open("./temp/binary_translation_matrix.txt", "x")

lines = []

# lines.append(["Hex", "Binary", "Dec", "u-Dec"])

for i in range(0, 256):
    lines.append([
        f"{i:X}", # hex
        f"{i:b}", # bin
        f"{i if i < 128 else i - 256}", # signed dec
        f"{i:d}" # unsigned dec
        ])
    
file.write(f'{"Hex".rjust(8)} | {"Binary".rjust(8)} | {"Dec".rjust(8)} | {"u-Dec".rjust(8)}')
file.write('-' * 41)


for line in lines:
    file.write(f'{line[0].rjust(8)} | {line[1].rjust(8)} | {line[2].rjust(8)} | {line[3].rjust(8)}')

file.close()