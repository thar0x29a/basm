arch("snes.cpu")

const REG_TM = 9000
fun val() { return 42 }

lda.b #{%00100001}
sta.w {REG_TM}

lda.b #{val()}
sta.w {1+2*3}