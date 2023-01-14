# Breaking the limits that were previously thought impossible. In the world's most competitive racing game, players take speedrunning to the extreme.
# Example usage:  race.py ./old_jangine ./new_jangine 3
import subprocess
import time
import sys
import os

cmd_ref = sys.argv[1]
cmd_new = sys.argv[2]
runs = int(sys.argv[3]) if len(sys.argv) >= 4 else 20
wins = 0

print("REF:", cmd_ref, "(binary to compare against)")
print("NEW:", cmd_new, "(new binary which is hopefully faster, i.e. negative difference)")
print("RUNS: ", runs)
print("  # NEW REF | Difference | Percentage | Values")

for run in range(runs):
    start_time = time.perf_counter()  # https://stackoverflow.com/a/62108504/2111778 :)
    p_ref = subprocess.Popen([cmd_ref, "-t"], stdout=subprocess.DEVNULL)  # started first so when in doubt ref is a bit faster
    p_new = subprocess.Popen([cmd_new, "-t"], stdout=subprocess.DEVNULL)

    # wait for the faster process to finish
    pid0 = None
    while pid0 != p_ref.pid and pid0 != p_new.pid:
        pid0, _ = os.wait()
        time0 = time.perf_counter() - start_time

    wins += (pid0 == p_new.pid)

    pid1 = None
    while pid1 != p_ref.pid and pid1 != p_new.pid:
        pid1, _ = os.wait()
        time1 = time.perf_counter() - start_time

    time_ref = time1 if pid1 == p_ref.pid else time0
    time_new = time1 if pid1 == p_new.pid else time0
    print(f"{run+1:3d} {wins:3d} {run + 1 - wins:3d} | {time_new - time_ref:9.2f}s | {time_new / time_ref:10.3f} | {time_new:.2f} - {time_ref:.2f}")
