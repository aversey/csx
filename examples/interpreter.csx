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

[set instrlist [fn []
  [set c [in]]
  [if [no [same c 10]] [pair c [instrlist]]]
]]
[set instr [fn [] [str [instrlist]]]]


{ The Program }

[set screenstr [fn [str]
  [set outstrat [fn [str i len] [if [no [same i len]] [do
    [if [same [str i] ["\\" 0]] [output "\\"]]
    [if [same [str i] ["\"" 0]] [output "\\"]]
    [out [str i]]
    [outstrat str [+ i 1] len]
  ]]]]
  [outstrat str 0 [len str]]
]]

[set screenname [fn [str]
  [set outstrat [fn [str i len] [if [no [same i len]] [do
    [if [same [str i] ["\\" 0]] [output "\\"]]
    [if [same [str i] [" " 0]] [output "\\"]]
    [if [same [str i] ["[" 0]] [output "\\"]]
    [if [same [str i] ["]" 0]] [output "\\"]]
    [out [str i]]
    [outstrat str [+ i 1] len]
  ]]]]
  [outstrat str 0 [len str]]
]]

[set writeeach [fn [l] [if l [do
  [write [head l]]
  [if [tail l] [do
    [output " "]
    [writeeach [tail l]]
  ]]
]]]]

[set write [fn [obj] [if
  [no obj] [output "[]"]
  [same [type obj] 'int] [outint obj]
  [same [type obj] 'str] [do [output "\""] [screenstr obj] [output "\""]]
  [same [type obj] 'pair] [do [output "["] [writeeach obj] [output "]"]]
  [same [type obj] 'name] [screenname [str obj]]
  [same [type obj] 'base] [output "<base>"]
  [same [type obj] 'int] [output obj]
  [same [type obj] 'fn] [output "<fn>"]
  [same [type obj] 'sx] [output "<sx>"]
]]]


[set linep [pair [] []]]

[set readcomment [fn [] [if
  [same [head [head linep]] ["}" 0]] [sethead linep [tail [head linep]]]
  [same [head [head linep]] ["{" 0]] [do
    [sethead linep [tail [head linep]]]
    [readcomment]
    [readcomment]
  ]
  [do
    [sethead linep [tail [head linep]]]
    [readcomment]
  ]
]]]

[set pairlist [fn [l] [if l [if [tail l]
  [pair [head l] [pairlist [tail l]]]
  [head l]
]]]]

[set readlist [fn [] [if
  [same [head [head linep]] ["]" 0]] [sethead linep [tail [head linep]]]
  [same [head [head linep]] [" " 0]] [do
    [sethead linep [tail [head linep]]]
    [readlist]
  ]
  [do
    [set res [readobj]]
    [pair res [readlist]]
  ]
]]]

[set readint [fn [num]
  [if [head linep] [if [< 47 [head [head linep]] 58] [do
    [set newnum [+ [* 10 num] [head [head linep]] -48]]
    [sethead linep [tail [head linep]]]
    [readint newnum]
  ] num] num]
]]

[set readname [fn []
  [if [head linep] [if [no [same [head [head linep]] [" " 0]]]
    [if [no [same [head [head linep]] ["[" 0]]] [if [no [same [head [head linep]] ["]" 0]]] [do
      [if [same [head [head linep]] ["\\" 0]] [sethead linep [tail [head linep]]]]
      [set res [head [head linep]]]
      [sethead linep [tail [head linep]]]
      [pair res [readname]]
    ]]]
  ]]
]]

[set readstr [fn []
  [if [head linep] [if [no [same [head [head linep]] ["\"" 0]]][do
    [if [same [head [head linep]] ["\\" 0]] [sethead linep [tail [head linep]]]]
    [set res [head [head linep]]]
    [sethead linep [tail [head linep]]]
    [pair res [readstr]]
  ] [sethead linep [tail [head linep]]]]]
]]

[set readobj [fn [] [if
  [same [head [head linep]] ["{" 0]] [do
    [sethead linep [tail [head linep]]]
    [readcomment]
    [readobj]
  ]
  [same [head [head linep]] [" " 0]] [do
    [sethead linep [tail [head linep]]]
    [readobj]
  ]
  [same [head [head linep]] ["[" 0]] [do
    [sethead linep [tail [head linep]]]
    [readlist]
  ]
  [same [head [head linep]] ["=" 0]] [if
    [same [head [tail [head linep]]] ["[" 0]] [do
      [sethead linep [tail [tail [head linep]]]]
      [pairlist [readlist]]
    ]
    [name [str [readname]]]
  ]
  [same [head [head linep]] ["-" 0]] [if
    [< 47 [head [tail [head linep]]] 58] [do
      [sethead linep [tail [head linep]]]
      [neg [readint 0]]
    ]
    [name [str [readname]]]
  ]
  [same [head [head linep]] ["'" 0]] [do
    [sethead linep [tail [head linep]]]
    [list quote [readobj]]
  ]
  [same [head [head linep]] ["\"" 0]] [do
    [sethead linep [tail [head linep]]]
    [str [readstr]]
  ]
  [< 47 [head [head linep]] 58] [readint 0]
  [name [str [readname]]]
]]]

[set read [fn [usercontext]
  [output "> "]
  [sethead linep [instrlist]]
  [set res [run [readobj] usercontext]]
  [write [head res]]
  [output newline]
  [read [tail res]]
]]

[output "-= CSX interpreter loaded =-" newline]
[output "========= Base functions: ========================================" newline]
[output "[set name value] | name will have the value value" newline]
[output "[set? name]      | check if name is set as some value" newline]
[output "[sethead p v]    | head of p will be changed to the v value" newline]
[output "[settail p v]    | head of p will be changed to the v value" newline]
[output "[pair head tail] | creates pair of head and value" newline]
[output "[head pair]      | returns head of pair" newline]
[output "[tail pair]      | returns tail of pair" newline]
[output "[quote x]        | returns unevaluated x" newline]
[output "[same a b]       | checks if a and b are the same thing" newline]
[output "[type x]         | returns type of x" newline]
[output "[do ...]         | runs statements returning value of the last one" newline]
[output "[fn args ...]    | creates function" newline]
[output "                 | you can make vararg function with pairs" newline]
[output "[sx args ...]    | same as fn but for macro" newline]
[output "[if a b ... c]   | executes b if a (for each a), else c" newline]
[output "[+ ...]          | sum of the arguments" newline]
[output "[* ...]          | product of the arguments" newline]
[output "[neg a]          | negate a" newline]
[output "[div a b]        | a / b" newline]
[output "[mod a b]        | a % b" newline]
[output "[< ...]          | checks if arguments are increasing" newline]
[output "[> ...]          | checks if arguments are decreasing" newline]
[output "[out ch]         | outputs character with ch code" newline]
[output "[in]             | null if input ended, else character code" newline]
[output "[name str]       | creates name from str string" newline]
[output "[str obj]        | creates str from name or list of codes" newline]
[output "[len obj]        | length of list or string" newline]
[output "[run x]          | runs the x expression" newline]
[output "[run x context]  | runs the x expression in given context" newline]
[output "                 | returns pair of value and new context" newline]
[output "[context]        | returns current context" newline]
[output "[newcontext]     | returns new base context" newline]
[output "==================================================================" newline]
[output "Have a good time!" newline]
[read [newcontext]]
