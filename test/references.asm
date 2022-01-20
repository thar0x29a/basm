var test = 4

fun foo(a, var b, const c, ref d) {
    var str = "" + a + ":" + b + ":" + c + ":" + d + ":" + [d]
    assert("1:2:3:test:4" == str)
}

foo(1,2,3,test)