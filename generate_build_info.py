import os
import sys
import subprocess
from datetime import datetime

if len(sys.argv) < 3:
    print("Error: Faltan argumentos. Uso: generate_build_info.py <build_dir> <source_dir>")
    sys.exit(1)

# Capturar las rutas enviadas desde CMake
build_dir = sys.argv[1]
source_dir = sys.argv[2]

home_dir = os.path.expanduser("~")
counter_file = os.path.join(home_dir, ".esp_build_counter.txt")
header_file = os.path.join(build_dir, "include", "version_info.h")

# Asegurar que la carpeta build/include exista
os.makedirs(os.path.dirname(header_file), exist_ok=True)

# Incrementador de build
counter = 1
if os.path.exists(counter_file):
    with open(counter_file, "r") as f:
        try:
            counter = int(f.read().strip()) + 1
        except ValueError:
            pass

with open(counter_file, "w") as f:
    f.write(str(counter))

# Obtener fecha y hora
now = datetime.now().strftime("%Y-%m-%d %H:%M:%S")

# Obtener información de Git.
def get_git_info(cmd):
    try:
        return subprocess.check_output(cmd, cwd=source_dir, stderr=subprocess.DEVNULL).decode().strip()
    except Exception:
        return "N/A"

git_branch = get_git_info(["git", "rev-parse", "--abbrev-ref", "HEAD"])
git_commit = get_git_info(["git", "rev-parse", "--short", "HEAD"])

# 4. Generar el header global unificado
header_content = f"""#ifndef VERSION_INFO_H
#define VERSION_INFO_H

#define BUILD_COUNTER       {counter}
#define BUILD_DATETIME      "{now}"
#define BUILD_GIT_BRANCH    "{git_branch}"
#define BUILD_GIT_COMMIT    "{git_commit}"

#endif // VERSION_INFO_H
"""

with open(header_file, "w") as f:
    f.write(header_content)

print(f"*** Build Counter Num: #{counter} | Time: {now} | Git: {git_branch}@{git_commit} ***")