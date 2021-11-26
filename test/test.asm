arch snes.cpu

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

    macro shout() {
      var d = 4

      print a, ".", b, ".", c, ".", d, "\n"
    }

    shout()
  }
  

  //baa.shout()
}

//foo.baa.shout()