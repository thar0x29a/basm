fun foo(a, var b, const c, ref d) {
    var str = "" + a + ":" + b + ":" + c + ":" + d + ":" + [d]
    assert("1:2:3:test4:4" == str)
}

var {"t"+"e"+"s"+"t"+4} = 4
foo(1,2,3,test4)