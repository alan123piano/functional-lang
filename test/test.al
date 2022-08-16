let add = fun a -> fun b -> a + b
in
let f = add in let g = f in
g (f 1 2) 2