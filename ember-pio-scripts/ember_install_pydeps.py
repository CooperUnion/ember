# https://docs.platformio.org/en/stable/scripting/examples/extra_python_packages.html

Import("env")

env.Execute("pip3 install -q -r lib/ember/ember-pio-scripts/requirements.txt")
