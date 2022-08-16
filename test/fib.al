(* Fibonacci *)
let eq = fun a -> fun b ->
        a = b
in
let fib = fun x ->
    if (eq x 0) then
        1
    else if (eq x 1) then
        1
    else
        (fib (x - 1)) + (fib (x - 2))
in
fib 5
