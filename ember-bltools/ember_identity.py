# Simple script to add a #define for EMBER_NODE_IDENTITY.

# From platformio.ini:

# 1. Try to get board_can_node and board_node_identity.
# 2. If neither are specified, raise an error.
# 3. Use these to add to CPPDEFINES to specify EMBER_NODE_IDENTITY,
# preferring board_node_identity over board_can_node if both are given.

# This means you can just specify board_can_node and that will be the
# node identity by default, or you can override it with board_node_identity.

import click

def log(msg, fg=None, bg=None, bold=False):
    click.echo(click.style("ember_identity.py: " + msg, fg=fg, bg=bg, bold=bold))

Import('env')

# Get options from platformio.ini
board_can_node      = env.GetProjectOption("board_can_node", default=None)
board_node_identity = env.GetProjectOption("board_node_identity", default=None)

# Check that at least one is set
if board_can_node is None and board_node_identity is None:
    error = "ember_identity.py: Error setting EMBER_NODE_IDENTITY: you must specify" \
            "either `board_can_node` or `board_node_identity` (the latter takes priority)."

    log(error, fg='red', bold=True)
    exit(-1)

# If both are set and they don't match, emit a note
if board_can_node is not None and board_node_identity is not None and board_can_node != board_node_identity:
    note = f"NOTE: board_can_node ({board_can_node}) does not match board_node_identity ({board_node_identity})"

    log(note, bg='blue', bold=True)

# Choose between the two
identity = board_can_node if board_node_identity is None else board_node_identity

log(f'Node identity is {identity}', fg='blue')

# Add a define that looks like -D EMBER_NODE_IDENTITY=\"identity\"
env.Append(CPPDEFINES=[
    ("EMBER_NODE_IDENTITY", f'\\\"{identity}\\\"') # \\\" unescapes as \"
])
