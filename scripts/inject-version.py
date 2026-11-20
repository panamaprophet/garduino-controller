import subprocess

Import("env")

cmd = ["git", "describe", "--tags", "--match", "v[0-9]*", "--dirty", "--always"]
cwd = env.Dir("#").abspath

print(f"✓ [firmware version] directory: {cwd}")

try:
    described = subprocess.check_output(cmd, cwd=cwd, stderr=subprocess.DEVNULL).decode().strip()
except (subprocess.CalledProcessError, FileNotFoundError):
    described = "unknown"

version = described[1:] if described.startswith("v") else f"0.0.0-{described}"

print(f"✓ [firmware version] version: {version}")

env.Append(
    BUILD_FLAGS=["-D FIRMWARE_VERSION=\\\"%s\\\"" % version]
)
