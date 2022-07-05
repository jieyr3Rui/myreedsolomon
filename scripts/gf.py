import random

primitive_polynomial_dict = {4: 0b10011,  # x**4  + x  + 1
                             8: (1 << 8) + 0b11101,  # x**8  + x**4  + x**3 + x**2 + 1
                             16: (1 << 16) + (1 << 12) + 0b1011,  # x**16 + x**12 + x**3 + x + 1
                             32: (1 << 32) + (1 << 22) + 0b111,  # x**32 + x**22 + x**2 + x + 1
                             64: (1 << 64) + 0b11011  # x**64 + x**4 + x**3 + x + 1
                             }
class GF:
    def __init__(self, w):
        self.w = w
        # total = 2 ** w -1
        self.total = (1 << self.w) - 1
        self.gflog = []
        self.gfilog = [1] # g(0) = 1
        self.make_gf_dict(self.w, self.gflog, self.gfilog)

    def make_gf_dict(self, w, gflog, gfilog):
        gf_element_total_number = 1 << w
        primitive_polynomial = primitive_polynomial_dict[w]
        for i in range(1, gf_element_total_number - 1):
            temp = gfilog[i - 1] << 1  # g(i) = g(i-1) * 2
            if temp & gf_element_total_number:  # 判断溢出
                temp ^= primitive_polynomial  # 异或本原多项式
            gfilog.append(temp)

        assert (gfilog[gf_element_total_number - 2] << 1) ^ primitive_polynomial
        gfilog.append(1)

        for i in range(gf_element_total_number):
            gflog.append(0)

        for i in range(0, gf_element_total_number - 1):
            gflog[gfilog[i]] = i

        sqrtN = int(2 ** (w/2))

        print("power")
        for i in range(sqrtN):
            s = ""
            for j in range(sqrtN):
                s += "%2x"%gfilog[i*sqrtN+j] + " "
            print(s)

        print("log")
        for i in range(sqrtN):
            s = ""
            for j in range(sqrtN):
                s += "%2x"%gflog[i*sqrtN+j] + " "
            print(s)

    def add(self, a, b):
        return (a ^ b) % self.total

    def sub(self, a, b):
        return (a ^ b) % self.total

    def mul(self, a, b):
        return self.gfilog[(self.gflog[a] + self.gflog[b]) % self.total]

    def div(self, a, b):
        # print((self.gflog[a] - self.gflog[b]) % self.total)
        return self.gfilog[(self.gflog[a] - self.gflog[b]) % self.total]
    


def alpha(k):
    res = 1
    for _ in range(k):
        res *= 2
        if res >= 256:
            res ^= 285
    return res

if __name__ == "__main__":
    gf = GF(8)
    
    # t = 0
    a = 32
    b = 142
    # c = gf.add(a, b)
    # d = gf.mul(a, b)
    print('%d + %d = %d' % (a, b, gf.add(a, b)))
    print('%d - %d = %d' % (a, b, gf.sub(a, b)))
    print('%d * %d = %d' % (a, b, gf.mul(a, b)))
    print('%d / %d = %d' % (a, b, gf.div(a, b)))
    # a, b = 4, 2
    # b_div_a = gf.div(b, a)
    # print("{} / {} = {}".format(b, a, b_div_a))
    # print(gf.mul(a, b_div_a))
    # print(alpha(9))
    # print(gf.mul(32,32))