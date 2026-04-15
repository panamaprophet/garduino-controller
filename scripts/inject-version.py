import subprocess
from datetime import datetime

Import("env")

cmd = ["git", "describe", "--tags", "--dirty", "--always"]
cwd = env.Dir("#").abspath

print (f"✓ [firmware version] directory: {cwd}")

today = datetime. today()
formatted_date = today.strftime("%Y%m%d")

prefix = "garduino-firmware"
version = subprocess.check_output(cmd, cwd=cwd, stderr=subprocess.DEVNULL)
formatted_version = "%s.%s.%s" % (prefix, version.decode().strip(), formatted_date)

print (f"✓ [firmware version] version: {formatted_version}")

env.Append(
    BUILD_FLAGS=["-D FIRMWARE_VERSION=\\\"%s\\\"" % formatted_version]
)