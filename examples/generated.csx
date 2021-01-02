[define map [fn [f l] [if [not l] []
  [pair [f [head l]] [map f [tail l]]]
]]]

[map [fn [n] [outnum n] [output 10]]
  [map [fn [n] [sum n 10]]
    [quote [1 2 3 4 5]]
  ]
]
