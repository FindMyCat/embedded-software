# FindMyCat - Outdoor Location Engine

Hardware: nrf9160-DK

Software Architecture:

![arch](./arch.png)



## MQTT Data format
When sending data over mqtt the following format is used to send a comma separated string to save as much data as possible.

latitude,longitude,satellites_tracked,accuracy
