let even = fix even ->
    fun odd -> fun n -> if n = 0 then true else (odd even) (n - 1)
in
let odd = fix odd ->
    fun even -> fun n -> if n = 0 then false else (even odd) (n - 1)
in
(even odd) 4