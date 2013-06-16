import subprocess, sys

command=['bash.exe', '-c']
command.append(" ".join(map(str, sys.argv[1:])))

if len(command)>2:
    try:
        p = subprocess.Popen(command, stdout=subprocess.PIPE)
        out, err = p.communicate()

        san = out.replace("\\", "/")

        sys.stdout.write(san)
    except Exception, e:
        sys.exit(1)

sys.exit(p.returncode)
