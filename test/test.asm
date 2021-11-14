arch snes.cpu

macro test() { 
  print "Hello world\n"
  print main, ":  ", a, ",", b,"\n"
}

main:
  const a = 12
  var b = 1

  test()