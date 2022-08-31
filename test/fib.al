(* Fibonacci *)
let fib = fix (fib : int -> int) -> fun x ->
    if x <= 2 then
        1
    else
        (fib (x - 1)) + (fib (x - 2))
in
fib 11
