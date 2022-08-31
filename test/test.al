let add = fun (a : int) -> fun (b : int) -> a + b
in
let f = add in let g = f in
g (f 1 2) 2