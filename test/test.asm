variable a = 0==1
variable b = 0>1
variable c = 0<1
variable d = 0>=1
variable e = 0<=1
variable f = 0!=1
variable g = (f=f)

var x = 20
while x > 0 {
  if (x > 16) {
    print x, " a\n"
  } else if x > 8 {
    print x, " b\n"
  } else {
    print x, " c\n"
  }

  x = x - 5
}


print "a is ", a, "\n"
print "b is ", b, "\n"
print "c is ", c, "\n"
print "d is ", d, "\n"
print "e is ", e, "\n"
print "f is ", f, "\n"
print "g is ", g, "\n"