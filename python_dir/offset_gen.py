offsets = [-9, -1, 7, -8, 8, -7, 1, 9]


def offset_bishop():
    for dist in range(7):
        for offset in offsets:
            if offset in [8, 1, -8, -1]:
                print(0, end=", ")
            else:
                print(offset * (dist + 1), end=", ")


def offset_rook():
    for dist in range(7):
        for offset in offsets:
            if offset not in [8, 1, -8, -1]:
                print(0, end=", ")
            else:
                print(offset * (dist + 1), end=", ")


def offset_queen():
    for dist in range(7):
        for offset in offsets:
            print(offset * (dist + 1), end=", ")


def offset_knight():
    offsets = [(-2, -1), (-2, 1), (-1, -2), (-1, 2), (1, -2), (1, 2), (2, -1), (2, 1)]
    for x, y in offsets:
        print(x + y * 8, end=", ")
    print()


if __name__ == "__main__":
    offset_queen()
    print()
    print()
    offset_rook()
    print()
    print()
    offset_bishop()
    print()
    print()
    offset_knight()
