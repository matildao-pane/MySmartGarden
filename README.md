# MySmartGarden

### Internet Of Things project by Matilde Mazzini


## Introdution
This project take inspiration from my garden. I developed a smart garden system, based on sensors and actuator installed in the area outside the house.

The devices can interface with a cloud application that shows the status of the devices and give the possibility to change it to the user.

## System Composition

The system is composed by two nodes:
a Gate node and a Watering node.

The Gate node is composed by a gate actuator and a photocell sensor.

The gate can be controlled by the user througth the interface. The gate can stay open for 10 seconds, then if the photocell sensor doesn't detect any presence, it will be closed automatically.

The irrigator node is composed by an irrigator actuator that controls the irrigator, a light sensor that collects data about the light and a humidity sensor that collects the humidity value of the soil.

The user can decide to set the watering system in manual or automatic mode.
If in automatic mode, the irrigator will start watering after the sunset, and only if the soil is not wet. 

The nodes are managed by the border router.

## Cooja Simulator
On the gate node the green led states if the gate is open or not.

On the irrigator node the green led represent the irrigator status, if the red led is on, it means that the humidity value detected is lower than the threshold, if the yellow led is on it means that the light value detected is lower than the threshold (the sunset has come). 
![cli](/doc/9.png)

## Cloud App
In the cloud application at the beginning the Coap Server registers all the resources of the nodes and start observing them.

The command line interface let the user interact with the system:
![cli](/doc/1.png)



## Run it
To run this project:

open a terminal and run the jar of the cloud app.

open the SmartGardenSimulation.csc in the cooja simulator.

open another terminal inside the folder rpl-border-route and run this command:

make TARGET=cooja connect-router-cooja

start the simulation
