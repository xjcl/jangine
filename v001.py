#!/usr/bin/python3


from time import sleep
import logging
import sys

logging.basicConfig(filename='janchess.log', level=logging.DEBUG)

def print(*args, **kwargs):
    __builtins__.print(*args, **kwargs)
    logging.info('>>> ' + ' '.join(str(m) for m in args))

def input(*args, **kwargs):
    ret = __builtins__.input(*args, **kwargs)
    logging.info(ret)
    return ret


for i in range(100):
    i = input()
    if 'xboard' in i:
        # print('feature myname="janchess"')
        print('feature done=1')
    if i.startswith('ping'):
        print(i.replace('i', 'o'))
    else:
        print('move e7e5')
    if 'q' == i:
        logging.shutdown()
        exit()
    sleep(.5)







    # if i.startswith('feature'):
    #     for item in i[len('feature'):]:
    #         key, value = item.split('=')
    #         print('accepted '+key)












