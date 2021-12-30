arch("snes.cpu")

//include("Font8x8.asm")

fun test(a) {
  var max = 20;
  while(a<max) {
    if(a==1) {
      notice(a, " first branch")
    }
    else if(a<10) {
      notice(a, " second branch") 
    }
    else {
      notice(a, " last branch")
    }
    a = a+1
  }
}

error( test(0) )