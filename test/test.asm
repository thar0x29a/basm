const a = "one"
constant foo = 13 - 8 * 2

macro test(
  b
) {
  include "test2.asm"
}

print "Hello World! ", 3+1*2+1, "\n"
print foo + 1000, "\n"
print test(" two ")