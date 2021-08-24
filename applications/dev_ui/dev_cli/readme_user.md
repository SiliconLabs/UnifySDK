# Dev CLI User's Guide

The developer CLI tool enables a quick and easy way to interact
with the special commands and increase the developing speed of protocol controllers
and other components. In addition, it is also a way to share custom tools.

## Installing Dev CLI

Follow the guide described in [README.md](../../../README.md).

## Running Dev CLI

Run `dev_cli --dev_cli.command help` to print a list of supported commands.

In the current state, (**9-28-2020**) the following three commands are implemented:

``` bash
./applications/dev_ui/dev_cli/dev_cli --dev_cli.command help
Commands:
    help  - Print commands and usage
    addsmartstartentry usage: [dev_clu.host *.port *.dsk *.include *.protocolunid *.unid] - Adding a smart start entry
    getsmartstartlist usage: [dev_cli.host *.port] - Get the smart start list
    removesmartstartentry usage: [dev_cli.host *.port *.dsk] - Removing smart start entry based on dsk key
```

### Examples

For example, deleting an entry from the smart start list may look like the
following:

``` bash
./dev_cli --dev_cli.command removesmartstartentry --dev_cli.dsk XXXXX

Removed smart start entry: XXXXX
```
