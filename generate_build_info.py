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
timestamp_header_file = os.path.join(build_dir, "include", "build_timestamp.h")

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
now_dt = datetime.now()
now = now_dt.strftime("%Y-%m-%d %H:%M:%S")

# Campos compatibles con struct tm de C
# tm_sec: 0-60, tm_min:0-59, tm_hour:0-23
# tm_mday:1-31, tm_mon: 0-11, tm_year: years since 1900
# tm_wday: days since Sunday (0 = Sunday), tm_yday: 0-365 (0 = Jan 1)
tm_sec = now_dt.second
tm_min = now_dt.minute
tm_hour = now_dt.hour
tm_mday = now_dt.day
# Python month is 1-12, struct tm expects 0-11
tm_mon = now_dt.month - 1
# struct tm year is years since 1900
tm_year = now_dt.year - 1900
# Python weekday(): Monday=0..Sunday=6; struct tm: Sunday=0..Saturday=6
tm_wday = (now_dt.weekday() + 1) % 7
# timetuple.tm_yday gives 1..366, struct tm expects 0..365
tm_yday = now_dt.timetuple().tm_yday - 1

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

# Generar header específico para `struct tm` con valores enteros sin ceros a la izquierda
timestamp_content = f"""#ifndef BUILD_TIMESTAMP_H
#define BUILD_TIMESTAMP_H

/* Valores pensados para inicializar directamente un `struct tm` */
#define BUILD_SEC  {tm_sec}
#define BUILD_MIN  {tm_min}
#define BUILD_HOUR {tm_hour}
#define BUILD_MDAY {tm_mday}
/* tm_mon: 0 = enero */
#define BUILD_MON  {tm_mon}
/* tm_year: años desde 1900 */
#define BUILD_YEAR {tm_year}
/* tm_wday: 0 = domingo */
#define BUILD_WDAY {tm_wday}
/* tm_yday: 0 = 1 de enero */
#define BUILD_YDAY {tm_yday}

#endif // BUILD_TIMESTAMP_H
"""

with open(timestamp_header_file, "w") as f:
    f.write(timestamp_content)

print(f"*** Build Counter Num: #{counter} | Time: {now} | Git: {git_branch}@{git_commit} ***")