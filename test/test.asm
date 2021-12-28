arch("snes.cpu")

//include("Font8x8.asm")
const a = "Hello"

blue {
  const b = "World"

  fun pill() {
    const c = "!"
    return a + " " + b + c;
  }
}

namespace red {
  const b = "Matrix"
  const pill = blue.pill
}

notice( blue.pill() )
warning( red.pill() )