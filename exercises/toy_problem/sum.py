import sys

sum = 0

with open(sys.argv[1]) as f:
    for line in f:
        sum = sum + float(line.split("|")[4])

print(sum)
