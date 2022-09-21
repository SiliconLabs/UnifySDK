# GMS User's Guide

GMS is an acronym for _Group Management Service_

## Description

The Group Management Service (GMS) a MQTT Client, is an application that manages the by-group
topic `ucl/by-group/`. It acts as a lookup entity that provides protocol controllers
 or other MQTT clients information about the interactions between
 groups, endpoints, and supported commands. It does not handle the topics that are
 used to execute group commands, for example
 `ucl/by-group/<group-id>/<Command>`, which is done by protocol controllers.

The GMS application does not persist any collected information. Instead, it collects
and publishes its data over the MQTT Framework.

Protocol controllers can run without the GMS application, which results in limited
functionality.

## Installation

For installing please refer to the general installation method in
[Getting started](../../doc/getting_started.md).

## Running the GMS

### Using Systemd Service

The best way to run the GMS is using the Systemd service that is installed with
the Debian installer. For more information, see the
[Unify Framework User guide](../../doc/unify_readme_user.md).

### Using Command Line

Alternatively, the GMS may be run by executing `uic-gms`. It is possible to
configure the MQTT server, database path, through command line options. For more
details, run `uic-gms --help`.

