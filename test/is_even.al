let even =
    fun odd -> fun n -> if n = 0 then true else odd (n - 1)
in
let odd =
    fun even -> fun n -> if n = 0 then false else even (n - 1)
in
(even odd) 1