# Evaluation

## Network

The critical network is the BLE one: the traffic on such network will be monitored and depends on number of active beacons and their update frequency.

## Preliminary estimates
- BLE beacon covers 100m range
- Doing some calculations the area of the circle drawn by the beacon is about 31k m^2
- A container 12m x 2,5m covers about 30 m^2
- Dividing the above areas we get around 1000 containers cover
- Then we can easily suppose that a stack of some containers
- The beacon transmits every minute
- Max 300 byte can be trasmitted
- We need 100 byte max
- Receiver deployed on lightpoles


## Power consumption

Power consumption is the most critical on BLE beacons as they have to be battery powered and operate continously for the duration of a containers' stay in the port. In order to evaluate the consumption we will need further information on how containers move within the port and their expected stay duration. Once this information is available IoT lab can be used to measure the consumption of the final system and provide a battery lifetime estimate, which will be evaluated and improved if necessary. Among the possible power saving techniques are:

- CPU sleep between timer interrupts
- Powering off the BLE module when not in use
- Integrating a motion sensor to limit pings when the container is stationary

## Response time

Power system does not have any user interaction that require a tight bound on response time since it is focused on measuring data for asynchronous analisys. Nonetheless excessive delays are to be avoided and the total latency from BLE ping to web dashboard will be measured and evaluated.
