var a = 1
var b = 1
var c = 1
var d = 1

namespace foo {
  var b = 2
  var c = 2
  var d = 2

  namespace baa {
    var c = 3
    var d = 3

    macro shout(e) {
      var d = 4
      b = b + e
      print a, ".", b, ".", c, ".", d, ".", e, "\n"
    }

    shout(1)
  }
  
  baa.shout(10)
}

foo.baa.shout(100)