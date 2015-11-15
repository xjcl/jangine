
import random
import time
import functools

# TEST WHAT IS FASTEST
# nested lists with double indexing
# nested lists with tuple indexing  # ??
# single list (with n*i+j indexing)
# nested dicts with double indexing
# single dict with tuple indexing



ran_data = [(random.randint(0,7), random.randint(0,7), random.randint(0,7)) for i in range(1000000)]


def timeit(func):
    @functools.wraps(func)
    def newfunc(*args, **kwargs):
        startTime = time.time()
        func(*args, **kwargs)
        elapsedTime = time.time() - startTime
        print('function [{}] finished in {} ms'.format(
            func.__name__, int(elapsedTime * 1000)))
    return newfunc



@timeit
def test(a, indexing='double'):
    if indexing == 'tuple':
        for x, y, z in ran_data:
            a[(x,y)] = z
    elif indexing == 'double':
        for x, y, z in ran_data:
            a[x][y] = z
    elif indexing == 'single':
        for x, y, z in ran_data:
            a[8*x+y] = z






ls = [0] * 64

ld = [
[0,0,0,0,0,0,0,0],
[0,0,0,0,0,0,0,0],
[0,0,0,0,0,0,0,0],
[0,0,0,0,0,0,0,0],
[0,0,0,0,0,0,0,0],
[0,0,0,0,0,0,0,0],
[0,0,0,0,0,0,0,0],
[0,0,0,0,0,0,0,0],
]

ds = dict()

dt = dict()

dd = {i : dict() for i in range(8)}

test(ls, indexing="single")
test(ld, indexing="double")  # winner

test(ds, indexing="single")
test(dt, indexing="tuple")   # loser
test(dd, indexing="double")


for x in (ls, ld, ds, dt, dd):
    print(x)



