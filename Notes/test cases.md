## notes, loops, and rands

120 1000, 240 1000, 0 500,
[120 500, 240 500] 4, 0 500,
[120 250, [240 100, 360 100] 3] 2, 0 500,
[1 1, 2 2] rand(3 6), 0 500,
rand(100 200) 1000, rand(200 300) 1000, 0 500,
120 rand(500 1000), 240 rand(750 1250), 0 500,
[120 500, 240 500] rand(2 5), 0 500,
[rand(100 150) rand(400 600), rand(200 250) rand(300 500)] rand(3 6), 0 500,

## patterns
pattern(1 2 1), 120 500, 240 500, 360 500, 0 500,
pattern(2 0 3 1), 120 250, 240 250, 360 167, 480 167, 600 167, 720 500, 0 500,
pattern(rand(1 3) rand(0 2) rand(1 3)), 120 500, 240 500, 360 500, 0 500,

## combinations
pattern(2 1 0), [rand(100 150) 500, rand(200 250) 500] rand(2 4), 0 500,
120 1, rand(120 130) rand(500 600), [pattern(1 rand(0 3) 0 0), rand(120 130) 500, rand(240 250) 500] rand(1 5), 0 500,

## linked randoms
lrand(a 200 1000) 1, lrand(a) 1
pattern(0 lrand(a 1 5)), pattern(0 lrand(a))

## edge cases
0 0,
20000 1,
[120 1] 0,
pattern(0 0 0), 120 500, 0 500,
100 100, 200 100, 300 100, 400 100, 500 100, [600 50, 700 50, 800 50] 10, 900 100, 1000 100, 1100 100, 1200 100, 0 500,
[100 100, [200 50, 300 50] 3, 400 100] 2, [500 200, [600 100, 700 100] 2] 3, 0 500,


## THESE ONES should fail

[rand(100 150) rand(400 600), rand(200 250) rand(300 500)]
[rand(100 150) rand(400 600), rand(200 250) rand(300 500)] pattern(1)

broken strings for testing resolveRand function

rand(200 100) 1
lrand(a 200 1000) 
lrand(a) 100, lrand(a 500 600) 1

