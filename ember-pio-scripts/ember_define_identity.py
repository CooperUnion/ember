# Simple script to #define EMBER_NODE_IDENTITY
from ember_identity import get_ember_identity

Import("env")

identity = get_ember_identity(env)
env.Append(CPPDEFINES=[("EMBER_NODE_IDENTITY", f'{identity}')])
