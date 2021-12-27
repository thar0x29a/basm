//arch("snes.cpu")

//include("Font8x8.asm")
const a = "Hello"

namespace space {
  const jam = "Bugs Bunny";
}

namespace {"foo"} {
  const b = "World"

  fun baa() {
    const c = "!"
    return a + b + c + space.jam;
  }
}

hard = foo.baa;
notice(hard());
warning(1+2*3)