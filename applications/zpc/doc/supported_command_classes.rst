Supported Z-Wave Command Classes
================================
.. meta::
  :description: Supported Z-Wave Command Classes
  :keywords: Z-Wave, CC, Command Classes, ZPC, Z-Wave PC Controller

.. warning::

  This documentation is a work in progress. We are actively working on adding all command classes supported here.


.. toctree::
  :maxdepth: 1
  :glob:
  
  ./command_classes/*


When starting ZPC from the command line, it will also display all the current supported command classes : 

.. code-block:: bash

  $ zpc
    | Command Class                      | Version | Support | Control | Security Level | Comment                                                                                                                                                                                                     |
    | ---------------------------------- | ------- | ------- | ------- | -------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
    | Alarm Sensor                       | 1       |         | x       | N/A            |                                                                                                                                                                                                             |
    | Association                        | 2       | x       | x       | Network Scheme |                                                                                                                                                                                                             |
    | Association Group Info (AGI)       | 3       | x       | x       | Network Scheme |                                                                                                                                                                                                             |
    | Barrier Operator                   | 1       |         | x       | N/A            |                                                                                                                                                                                                             |
    | Basic                              | 2       |         | x       | N/A            |                                                                                                                                                                                                             |
    | Battery                            | 3       |         | x       | N/A            | Control Part is auto-generated.                                                                                                                                                                             |
    | Binary Sensor                      | 1       |         | x       | N/A            | Control Part is auto-generated.                                                                                                                                                                             |
    | Binary Switch                      | 2       |         | x       | N/A            |                                                                                                                                                                                                             |
    | Central Scene                      | 3       |         | x       | N/A            | Partial control: key attributes are not displayed in the UI.                                                                                                                                                |
    | Color Switch                       | 3       |         | x       | N/A            | Partial Control:<br>1. Durations are ignored<br>2. No support for Start/Stop Level Change<br>3. No support for other Color Component IDs than 0,1,2,3,4                                                     |
    | Configuration                      | 4       |         | x       | N/A            |                                                                                                                                                                                                             |
    | Device Reset Locally               | 1       | x       | x       | Network Scheme |                                                                                                                                                                                                             |
    | Door Lock                          | 4       |         | x       | N/A            |                                                                                                                                                                                                             |
    | Firmware Update                    | 7       | x       | x       | Network Scheme |                                                                                                                                                                                                             |
    | Inclusion Controller               | 1       | x       | x       | Unencrypted    |                                                                                                                                                                                                             |
    | Indicator                          | 3       | x       | x       | Network Scheme |                                                                                                                                                                                                             |
    | Manufacturer Specific              | 2       | x       | x       | Network Scheme |                                                                                                                                                                                                             |
    | Meter                              | 6       |         | x       | N/A            | Partial control: <br>1. Meter Reset Command is not supported due to missing dotdot ZCL equivalent functionality.                                                                                            |
    | Multi Channel                      | 4       |         | x       | N/A            |                                                                                                                                                                                                             |
    | Multi Channel Association          | 3       | x       | x       | Network Scheme |                                                                                                                                                                                                             |
    | Multi Command                      | 1       | x       |         | Unencrypted    |                                                                                                                                                                                                             |
    | Multilevel Sensor                  | 11      |         | x       | N/A            | Partial control: <br>1. Not all scales are supported<br>2. Not all types are shown in the UI.                                                                                                               |
    | Multilevel Switch                  | 4       |         | x       | N/A            |                                                                                                                                                                                                             |
    | Notification                       | 8       |         | x       | N/A            | Partial Control: <br>1. No Push/Pull discovery is done.<br>2. No Pull sensor support. <br>3. Unknown types are not supported. <br>4. No Regular probing is done.                                            |
    | Powerlevel                         | 1       | x       |         | Network Scheme |                                                                                                                                                                                                             |
    | Scene Activation                   | 1       |         | x       | N/A            |                                                                                                                                                                                                             |
    | Scene Actuator Configuration       | 1       |         | x       | N/A            | No interview performed automatically.                                                                                                                                                                       |
    | Scene Controller Configuration     | 1       |         | x       | N/A            |                                                                                                                                                                                                             |
    | Security 0                         | 1       | x       | x       | Unencrypted    |                                                                                                                                                                                                             |
    | Security 2                         | 1       | x       | x       | Unencrypted    |                                                                                                                                                                                                             |
    | Sound Switch                       | 2       |         | x       | N/A            | You can control the specific tone play with the ON_OFF_ON_TIME ZCL attribute. See code or python CTT script for details.                                                                                    |
    | Supervision                        | 2       | x       | x       | Unencrypted    |                                                                                                                                                                                                             |
    | Thermostat Mode                    | 3       |         | x       | N/A            | Partial Control: Not all modes can be set                                                                                                                                                                   |
    | Thermostat Setpoint                | 3       |         | x       | N/A            | Partial Control: <br>1. No discovery of ambiguous types in v1-v2 <br>2. Only a few setpoints can be configured. <br>3. Precision/size fields in the set are determined <br>automatically by the controller. |
    | Time                               | 1       | x       |         | Unencrypted    |                                                                                                                                                                                                             |
    | Transport Service                  | 2       | x       | x       | Unencrypted    |                                                                                                                                                                                                             |
    | User Code                          | 2       |         | x       | N/A            | Partial Control                                                                                                                                                                                             |
    | Version                            | 3       | x       | x       | Network Scheme |                                                                                                                                                                                                             |
    | Wake Up                            | 3       |         | x       | N/A            |                                                                                                                                                                                                             |
    | Z-Wave Plus Info                   | 2       | x       | x       | Unencrypted    |                                                                                                                                                                                                             |

