# Simple script to add a #define for EMBER_NODE_IDENTITY.

from SCons.Script import ARGUMENTS

def get_ember_identity(env):
    identity = env.GetProjectOption('board_node_identity', default=None)
    if identity is None:
        if env.GetProjectOption('custom_ember_do_not_use_can_node_as_identity', default=0) != 0:
            print("Error: board_node_identity must be set in platformio.ini - "\
                "(will not use board_can_node as identity because custom_ember_do_not_use_can_node_as_identity is set)")
            exit(-1)

        identity = env.GetProjectOption('board_can_node', default=None)
        if identity is None:
            print("Error: board_node_identity or board_can_node must be set in platformio.ini")
            exit(-1)

        if int(ARGUMENTS.get("PIOVERBOSE", 0)):
            print(f"Using board_can_node as node identity: {identity}")

    env.Append(CPPDEFINES=[("EMBER_NODE_IDENTITY", f'{identity}')])

    return identity

if __name__ == "__main__":
    Import('env')
    get_ember_identity(env)
