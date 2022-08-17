let add = fun a -> fun b ->
    let ap = a + 1 in
    let bm = b - 1 in
    a + b
in
add 3 (-10)