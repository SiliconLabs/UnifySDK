# Name and location service User's Guide

NAL is an acronym for _Name and Location service_.

## Description

The Name and Location Service is a helper MQTT component that allows to perform book-keeping on text names and locations that have been assigned. This functionality allows IoT Services to assign and readback a Name and a Location for each node/endpoint.
It subscribes to 2 topics:

* `ucl/by-unid/+/State`
* `ucl/by-unid/+/+/#`

But, actually, it processes such topics:

* `ucl/by-unid/<UNID>/State` - to add/delete node
* `ucl/by-unid/<UNID>/<endpoint_id>/#` - to detect a new endpoint
* `ucl/by-unid/<UNID>/<endpoint_id>/NameAndLocation/Commands/WriteAttributes` - to update Name or Location attribute
* `ucl/by-unid/<UNID>/<endpoint_id>/Basic/Attributes/LocationDescription/<Desired/Reported>` - to implement a proxying over Protocol Controller

NAL provides persistent storage for its data using SQLite database.

## Installation

Please refer to [Unify SDK User guide](../../doc/readme_user.md).

## Running the NAL 

### Using Systemd Service

The best way to run the NAL is using the Systemd service that is installed with
the Debian installer. For more information, see [Unify SDK User guide](../../doc/readme_user.md).

### Using Command Line

Alternatively, the NAL may be run by executing `nal`. It is possible to
configure the MQTT broker Hostname or IP, NAL database path, through command line 
options. For more details about the options, run `uic-nal --help`.


## How to use it

### How to create a node with two endpoints, update it's Name and Location attributes and delete a node

``` bash
username@hostname:~ $ mosquitto_pub -h localhost -p 1883 -t "ucl/by-unid/node_test/State" -m 'some_dummy_data' # create a node with unid "node_test", endpoint 0
username@hostname:~ $ mosquitto_pub -h localhost -p 1883 -t "ucl/by-unid/node_test/ep1/Basic/Attributes/PowerSource/Reported" -m '{"any_json":"payload"}' # add a node with unid "node_test", endpoint 1
username@hostname:~ $ mosquitto_pub -h localhost -p 1883 -t "ucl/by-unid/node_test/ep1/NameAndLocation/Commands/WriteAttributes" -m '{"Name":"new_name"}' # write Name attribute for node with unid "node_test", endpoint 1
username@hostname:~ $ mosquitto_pub -h localhost -p 1883 -t "ucl/by-unid/node_test/ep1/NameAndLocation/Commands/WriteAttributes" -m '{"Location":"new_location"}' # write Location attribute for node with unid "node_test", endpoint 1
username@hostname:~ $ mosquitto_pub -h localhost -p 1883 -t "ucl/by-unid/node_test/State" -m '' # remove all endpoints with unid "node_test"
```

### How to test a proxying feature

``` bash
username@hostname:~ $ mosquitto_pub -h localhost -p 1883 -t "ucl/by-unid/node_test/State" -m 'some_dummy_data' # create a node with unid "node_test", endpoint 0
username@hostname:~ $ mosquitto_pub -h localhost -p 1883 -t "ucl/by-unid/node_test/ep0/Basic/Attributes/LocationDescription/Desired" -m '{"value":"Rooftop"}' # after receiving a "LocationDescription" message - the node is "proxied" and couldn't perform the Location attribute update without Protocol Controller
```

Now a user wants to update a Location attribute by publishing to "WriteAttributes" topic:

``` bash
username@hostname:~ $ mosquitto_pub -h localhost -p 1883 -t "ucl/by-unid/node_test/ep0/NameAndLocation/Commands/WriteAttributes" -m '{"Location":"updated_location"}'
```

But the NAL service doesn't update anything, it publishes a message to `ucl/by-unid/node_test/ep0/Basic/Commands/WriteAttributes` topic and waits a command to change Location attribute from the Protocol Controller:

``` bash
username@hostname:~ $ mosquitto_sub -v -h localhost -p 1883 -t '#'
ucl/by-unid/node_test/State some_dummy_data # published by user
ucl/by-unid/node_test/ep0/NameAndLocation/Attributes/Name/Desired {"value":""} # published by NAL service
ucl/by-unid/node_test/ep0/NameAndLocation/Attributes/Name/Reported {"value":""} # published by NAL service
ucl/by-unid/node_test/ep0/NameAndLocation/Attributes/Location/Desired {"value":""} # published by NAL service
ucl/by-unid/node_test/ep0/NameAndLocation/Attributes/Location/Reported {"value":""} # published by NAL service
ucl/by-unid/node_test/ep0/NameAndLocation/SupportedCommands {"value":["WriteAttributes"]} # published by NAL service
ucl/by-unid/node_test/ep0/Basic/Attributes/LocationDescription/Desired {"value":"Rooftop"} # published by Protocol Controller, after this the node is "proxied"
ucl/by-unid/node_test/ep0/NameAndLocation/Attributes/Location/Desired {"value":"Rooftop"} # published by NAL service
ucl/by-unid/node_test/ep0/NameAndLocation/Commands/WriteAttributes {"Location":"updated_location"} # published by user, the user expects the Location attribute will be changed
ucl/by-unid/node_test/ep0/Basic/Commands/WriteAttributes {"LocationDescription":"updated_location"} # published by NAL service, NAL waits for responce from the Protocol controller
ucl/by-unid/node_test/ep0/Basic/Attributes/LocationDescription/Desired {"value":"updated_location"} # published by Protocol Controller, the NAL should change the Desired Location attribute value only after receiving such message
ucl/by-unid/node_test/ep0/NameAndLocation/Attributes/Location/Reported {"value":"updated_location"} # NAL reports the Reported Location attribute was changed
ucl/by-unid/node_test/ep0/Basic/Attributes/LocationDescription/Reported {"value":"updated_location"} # published by Protocol Controller, the NAL should change the Reported Location attribute value only after receiving such message
ucl/by-unid/node_test/ep0/NameAndLocation/Attributes/Location/Reported {"value":"updated_location"} # NAL reports the Reported Location attribute was changed
```
