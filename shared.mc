include "buffers.mc"

let loopFn : all a. a -> (Int -> a -> a) -> a = lam v. lam f.
  recursive let work = lam i. lam v.
    let vnext = f i v in
    work (addi i 1) vnext
  in work 1 v

let printFloatBuffer : Int -> () = lam id.
  let buf = _loadBuffer id in
  let printTsv = lam tsv.
    match tsv with (ts, value) in
    join [int2string ts, " ", float2string (unsafeCoerce value)]
  in
  printLn (strJoin "\n" (map printTsv buf));
  exit 0

let printFloatDistributionBuffer : Int -> () = lam id.
  let buf = _loadBuffer id in
  let printTsv = lam tsv.
    match tsv with (ts, dist) in
    let dist : Dist Float = unsafeCoerce dist in
    match distEmpiricalSamples dist with (samples, weights) in
    recursive let work = lam samples. lam weights.
      match (samples, weights) with ([s] ++ samples, [w] ++ weights) then
        printLn (join [float2string s, " ", float2string w]);
        work samples weights
      else ()
    in work samples weights
  in
  iter printTsv buf;
  exit 0

let handleOptions : Options -> () = lam options.
  if neqi options.printFloat (negi 1) then
    printFloatBuffer options.printFloat
  else if neqi options.printDist (negi 1) then
    printFloatDistributionBuffer options.printDist
  else ()

let cmpTsv : (Int, Float) -> (Int, Float) -> Int = lam l. lam r.
  if gtf l.1 r.1 then 1
  else if ltf l.1 r.1 then negi 1
  else 0

-- Finds the median among a given sequence of observations. If there is an even
-- number of observations, the median is given the minimum timestamp among the
-- two considered values.
let median : [(Int, Float)] -> (Int, Float) = lam obs.
  let n = length obs in
  let obs = sort cmpTsv obs in
  if eqi (modi n 2) 0 then
    let fst = divi n 2 in
    match get obs fst with (ts1, v1) in
    match get obs (addi fst 1) with (ts2, v2) in
    (mini ts1 ts2, divf (addf v1 v2) 2.0)
  else
    get obs (divi n 2)
