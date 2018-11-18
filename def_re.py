import subprocess


def tokens():
    container_id = 'f88f372f7f1c'
    o = subprocess.check_output(['docker', 'exec', container_id, 'sh', '/getandclean.sh'])
    return set(o.split("\n"))

print(tokens())

