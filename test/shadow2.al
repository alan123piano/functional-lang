let dbl = fix dbl -> fun n -> if n <= 0 then 0 else (fun dbl -> 2 + dbl (n - 1)) 0
in
dbl 5