arch("snes.cpu")

//include("Font8x8.asm")

fun test(a) {
  if(a==1) {
    notice(a, " first branch")
  }
  else if(a<10) {
    notice(a, " second branch")
    return true; 
  }
  else {
    notice(a, " last branch")
  }
  warning("has not returned");
}

error( test(9) )