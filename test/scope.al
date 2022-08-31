let f = (let a = 1 in (fun (x : int) -> (x + a)))
in
f