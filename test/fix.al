let fix_ = fun f -> let x = f x in x in
(fix_ (fun x -> if x <= 0 then 0 else f (x - 1) + 2)) 10