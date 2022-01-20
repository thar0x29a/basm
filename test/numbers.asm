const dec = -10
const bin = %10
const hex = $10

const bin2 = 0b101
const oct = 0o101
const hex2 = 0xaa

const dec2 = 123'456'789

assert(2==bin)
assert(5==bin2)

assert(65==oct)
assert(-10==dec)
assert(123456789==dec2)

assert(16==hex)
assert(170==hex2)