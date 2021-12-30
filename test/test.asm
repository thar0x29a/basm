arch("snes.cpu")


const REG_BGMODE = $2105
const cool = 10

fun test() {
  //if(cool == 1) { return 1337 }
  //else if(cool == 2) { return 9000 }
  //else { return false }
  return true
}


// Setup Video
  lda.b #%00001000 // DCBAPMMM: M = Mode, P = Priority, ABCD = BG1,2,3,4 Tile Size
  sta.w REG_BGMODE // $2105: BG Mode 0, Priority 1, BG1 8x8 Tiles

  lda.b #%00000001 // Enable BG1

  sta.w test()
  //sta.w {(1+2*3)}
Loop:
  jmp Loop