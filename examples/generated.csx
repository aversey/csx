{ Base Utilities }

[set no [fn [x] [same x []]]]
[set id [fn [arg] arg]]
[set list [fn args args]]

[set catrev [fn [a b] [if a [catrev [tail a] [pair [head a] b]] b]]]
[set rev [fn [l] [catrev l []]]]
[set cat [fn [a b] [catrev [rev a] b]]]

[set map [fn [f l] [if l [pair [f [head l]] [map f [tail l]]]]]]
[set reduce [fn [f l] [if [no l] [] [if [no [tail l]] [head l]
  [f [head l] [reduce f [tail l]]]
]]]]

[set - [fn =[a rest] [+ a [reduce + [map neg rest]]]]]


{ Input-Output }

[set newline [str [list 10]]]

[set outint [fn [n]
  [set zero 48]
  [set minus 45]
  [if [< n 0]
    [do [out minus] [outint [neg n]]]
    [if [< n 10]
      [out [+ zero n]]
      [do
        [outint [div n 10]]
        [out [+ zero [mod n 10]]]
      ]
    ]
  ]
]]

[set outstr [fn [str]
  [set outstrat [fn [str i len] [if [no [same i len]] [do
    [out [str i]]
    [outstrat str [+ i 1] len]
  ]]]]
  [outstrat str 0 [len str]]
]]

[set output [fn objs [map [fn [obj] [if
  [same [type obj] 'int] [outint obj]
  [same [type obj] 'str] [outstr obj]
]] objs]]]

[set instr [fn []
  [set instract [fn []
    [set c [in]]
    [if [no [same c 10]] [pair c [instract]]]
  ]]
  [str [instract]]
]]


{ The Program }

[output "Hello, I am Casey Shawn Exton.  What is your name?" newline]
[output "> "]
[set name [instr]]
[output "Nice to meet you, " name "." newline]
[output "Your name is " [len name] " characters long." newline]
[output "I have to go.  Goodbye!" newline]
