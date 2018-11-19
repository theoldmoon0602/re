from flag import *
import sys
import subprocess

sys.stdout.write("Input your query: ")
sys.stdout.flush()
query = sys.stdin.readline().strip()

if len(query) == 0:
    sys.exit()

r = subprocess.check_output(["./re", query, "./re.txt"], timeout=1)
try:
    out = r.decode().split("\n")[0].strip()[:38]
except UnicodeDecodeError:
    out = "".join(list(map(chr, r)))[:38]


sys.stdout.write("result is: " + out + "\n")
sys.stdout.flush()

if "givemeaflag" in out:
    sys.stdout.write("...and flag is: " + flag +"\n")
    sys.stdout.flush()

if len(out) > 0:
    with open("./defense-flags.txt", "a") as f:
        f.write(out + "\n")

