Import("env")

from ember_identity import get_ember_identity

identity = get_ember_identity(env)

env.AddCustomTarget(
    "ota",
    "$BUILD_DIR/${PROGNAME}.bin",
    f"python3 lib/ember/ember_bl_updater.py --bin $SOURCE --target {identity}"
)
