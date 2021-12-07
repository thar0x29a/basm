namespace {("f"+"o"+"o")} {
  macro magic(a, const b, eval c) {
    // parameters only valid in this function
    const DISPLAY = 1
    const POWER = 0x2
    const RAINBOW = 0xA

    {"cloud"} = 0xff

    print a, ", ", b, ", ", {c}, " (", c, ")\n"
  }
}

foo.magic(1, 2, RAINBOW)