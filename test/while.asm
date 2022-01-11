var i = 0

while(i<50) {
    i = i+1

    if(i==5) { continue }
    else if(i==18) { break }

    assert(i!=5)
    assert(i<18)
}