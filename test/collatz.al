let collatz =
    fix (collatz : int -> int) ->
        fun n ->
            if n = 1 then
                0
            else
                if n % 2 = 0 then
                    1 + collatz (n / 2)
                else
                    1 + collatz (3 * n + 1)
in
collatz 97
(* collatz 10 = 6 *)