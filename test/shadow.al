let dbl = fix dbl -> fun n -> if n <= 0 then 0 else let dbl = 2 in 2 + dbl (n - 1)
in
dbl 5