
from tkinter import N


def main():
    x, Px = 8, 0x11d
    N, sqrtN = 2 ** x, int(2 ** (x/2))
    
    p, l = [0] * N, [0] * N
    n = 1
    for i in range(N):
        p[i] = n
        l[n] = i
        n*=2
        if n >= N:
            n = n ^ Px
    l[1] = 0

    print("power")
    for i in range(sqrtN):
        s = ""
        for j in range(sqrtN):
            s += "%2x"%p[i*sqrtN+j] + " "
        print(s)

    print("log")
    for i in range(sqrtN):
        s = ""
        for j in range(sqrtN):
            s += "%2x"%l[i*sqrtN+j] + " "
        print(s)
    return

if __name__ == "__main__":
    main()