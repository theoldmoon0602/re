import subprocess


def tokens():
    container_id = subprocess.check_output("docker ps | grep re | awk '{print $1}'", shell=True).strip()
    o = subprocess.check_output(['docker', 'exec', container_id, 'sh', '/getandclean.sh']).strip()
    return set(o.split("\n"))

