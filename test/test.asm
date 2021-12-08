namespace {("f"+"oo")} {
  { "label" }:
  { ("left"+"side") } = "works"

  macro magic(a, const b, ref c) {
    // parameters only valid in this function
    const DISPLAY = 1
    const POWER = 0x2
    const RAINBOW = 0xA

    return (" "+a+", "+b+", "+[c]+" ("+c+") "+label+", "+leftside+"\n")
  }
}

print RAINBOW, foo.magic(1, 2, RAINBOW)