//arch("snes.cpu")

//include("Font8x8.asm")

const hello = "world";
var catmakes = "meow"

fun foo() {
  notice("Foo? Bah! ", hello, " ", catmakes);
}
fun foo(var num) {
  warning(num, " is alot to foo!");
}

const test = pikachu = hello

test();