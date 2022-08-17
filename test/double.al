let dbl = fix dbl -> fun x -> if x = 0 then 0 else dbl (x - 1) + 2
in
dbl 10