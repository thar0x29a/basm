//arch("snes.cpu")

//include("Font8x8.asm")

const hello = "world";

fun solution() {
  return 42;
}

fun foo() {
  var catmakes = "meow"
  warning("Foo? Bah! ", hello, " ", catmakes);
}

fun foo(var num) {
  warning(num, " is alot to foo!");
}

fun refback() {
  return foo;
}

const test = pikachu = foo

test(solution());
//pikachu();

var urgs = refback();
urgs(9001);

print("END\n\n");