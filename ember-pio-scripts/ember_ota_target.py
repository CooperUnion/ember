Import("env")

from ember_identity import get_ember_identity

identity = get_ember_identity(env)

# https://community.platformio.org/t/access-upload-port-from-platformio-ini-from-extra-script-py/1350/3
can_dev = env.get("UPLOAD_PORT")
if can_dev is None:
    can_dev = "can0"

env.AddCustomTarget(
    "ota",
    "$BUILD_DIR/${PROGNAME}.bin",
    f"python3 lib/ember/ember_bl_updater.py --bin $SOURCE --target {identity} --iface {can_dev}"
)
