# Supported command classes documentation guide

We use .rst instead of .md to have better control over the rendered page.
It allows us to have a local table of contents and a better control over tables.

Use the cc_template to write new documentation and put it in the command_class folder. The index should be updated automatically.

## How to write

You should set the current version supported for each command classes in the header. If they are specifics to the command class, you can put it here (see thermostat_operating_state.rst, thermostat_setpoint.rst or user_credential.rst).

**No need to specify the prefix in the attribute names. ``ATTRIBUTE_COMMAND_CLASS_SOUND_SWITCH_VERSION`` will be referenced at ``VERSION``**

### Interview process

A good start is the function that is bound to the ``VERSION`` attribute : 

```c
  attribute_store_register_callback_by_type(
    &zwave_command_class_sound_switch_on_version_attribute_update,
    ATTRIBUTE_COMMAND_CLASS_SOUND_SWITCH_VERSION);
```

```c
static void zwave_command_class_sound_switch_on_version_attribute_update(
  attribute_store_node_t updated_node, attribute_store_change_t change)
{
  // ...

  // Attribute initialisation order is important here since it determines in 
  // which order the GET commands will be sent
  const attribute_store_type_t attributes[]
    = {
       ATTRIBUTE_COMMAND_CLASS_SOUND_SWITCH_TONES_NUMBER,
       ATTRIBUTE_COMMAND_CLASS_SOUND_SWITCH_CONFIGURED_DEFAULT_VOLUME,
       ATTRIBUTE_COMMAND_CLASS_SOUND_SWITCH_CONFIGURED_DEFAULT_TONE_IDENTIFIER,
       ATTRIBUTE_COMMAND_CLASS_SOUND_SWITCH_TONE_PLAY};

  attribute_store_add_if_missing(endpoint_node,
                                 attributes,
                                 COUNT_OF(attributes));
}
```

Check to which Get function those attributes are bound. It'll give you the interview order.

Note that sometimes only one attribute is created, and the other ones are created in the report for this attribute (e.g. in thermotsat setpoint).

### Command Class Commands

#### Set commands

Check how the attributes are fetched in the function (either desired/reported or desired or else reported). Z-Wave Set Field are the field name in the Z-Wave specification.

#### Get commands

The template have ``.. _link-to-command:``, this is because .rst doesn't generate links for each section automatically. The Get function can be referenced in the interview process section, that's why you might want to define a custom anchor before them.

A get function might not have any argument, so you can skip the binding table.

#### Report commands

Z-Wave Command Attribute are the field name in the Z-Wave specification. You can also specify the attribute store implementation to help the user to visualize how it is constructed (e.g Thermostat Operating State Logging Report).

### Unify cluster

#### UAM files

All the uam files are located under ``applications/zpc/components/dotdot_mapper/rules``. Look for the one that is applied to the command class you are using. Write a command to explain how the cluster is used.

#### Bindings

Refer to the ID of attribute in the attribute store to know which is bound to which. Also look in the code for ``attribute_store_register_callback_by_type_and_state`` and ``attribute_store_register_callback_by_type`` to see if those mapping are done directly in C (e.g in thermostat setpoint).

Write a small comment if the cluster attribute is in read only or can be written (``writable="true"``).

#### Command actions

You can use the Dev UI to see which MQTT topics are send when you are interacting with the command classes.

You can start the dev-gui-api from the command line to see the MQTT topics :

``` bash
> cd applications/dev_ui/dev_gui
> npm install (you might need node 18 on your local installation)
> sudo service uic-dev-gui-api stop 
> npm run start-api
```

The MQTT reference documentation can be useful also : `components/uic_dotdot/zap-generated/readme_ucl_mqtt_reference.md`

This folder also contains the callback called when the commands are invoked : `components/unify_dotdot_attribute_store/src`. This can be used to see which attributes are modified when invoking a command.

