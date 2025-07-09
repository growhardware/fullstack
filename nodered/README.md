# Garden-brain

Computer interface between medulla machines and huertos-API.

### About

This is your project's README.md file. It helps users understand what your
project does, how to use it and anything else they may need to know.

## correr con este comando

docker run -it -p 1880:1880 --name garden-brain nodered/node-red

## ejemplo de id contenedor creado

3007764a5ff192a2dacb8814459e53ac4ddc9032b9390d6c7e74f85d44afbd07

## para restartear el contenedor

docker restart 3007764a5ff192a2dacb8814459e53ac4ddc9032b9390d6c7e74f85d44afbd07

## TODO!

# ver de hacer un docker file para incluir en un contenedor una imagen que tenga node red mas los dos siguiente packages

# https://flows.nodered.org/node/node-red-contrib-sails

# (se puede agregar desde > Palette Manager (alt + shift + p) > node-red-contrib-sails )

# https://flows.nodered.org/node/node-red-node-serialport

# (se puede agregar desde > Palette Manager (alt + shift + p) > node-red-node-serialport )

# se subiría a docker hub como el stack de growhardware (garden brain) basico

## el siguiente NO funcionó

## docker run --name garden-brain -d nodered/node-red
