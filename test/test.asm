variable a = 0==1
variable b = 0>1
variable c = 0<1
variable d = 0>=1
variable e = 0<=1
variable f = 0!=1
variable g = (f=f)

variable x = 20

//while x > 0 {
  if x > 16 {
    print x, "a "
  } else if x > 8 {
    print x, "b "
  } else {
    print x, "c "
  }

  x = x - 1
//}

x = a - 1

print "a is ", a, "\n"
print "b is ", b, "\n"
print "c is ", c, "\n"
print "d is ", d, "\n"
print "e is ", e, "\n"
print "f is ", f, "\n"
print "g is ", g, "\n"