[set map [fn [f l] [if [same l []] []
  [pair [f [head l]] [map f [tail l]]]
]]]

[map [fn [n] [sum n 10]]
  [quote [1 2 3 4 5]]
]
