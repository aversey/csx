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

[set readname [fn [] [output [str [head linep]] newline]
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

[set readobj [fn [] [output [str [head linep]] newline] [if
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
  [set readed [readobj]]
  [write readed] [output newline]
  [set res [run readed usercontext]]
  [write [head res]]
  [output newline]
  [read [tail res]]
]]

[output "-= CSX interpreter loaded =-" newline]
[read [newcontext]]
