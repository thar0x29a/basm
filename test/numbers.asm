const dec = 10
const bin = %10
const hex = $10

const bin2 = 0b101
const oct = 0o101
const hex2 = 0x101

assert(2==bin)
assert(5==bin2)

assert(65==oct)
assert(10==dec)

assert(16==hex)
assert(257==hex2)