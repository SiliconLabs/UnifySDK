# uic_mapper_testninja
Application for mapper files testing.
 
Application creates attribute tree, load mapper rules from UAM file, activates mapper rules and tests if
rules applied correctly.

Attribute tree, UAM file path and test data described in JSON config file

``` bash
uic_mapper_test ../components/uic_attribute_mapper/test/uam_test_v4.json
```

returns value = 0,  if all test passed

returns value > 0,  if test failed, value equals to number of failed tests

returns value < 0,  if data processing error took place

Config file example:
```json
{
    "uam_file": "../components/uic_attribute_mapper/test/rules/TemperatureMeasurement.uam",
    "tree_data": [{node_data_object}],
    "test_data": [{test_data_object}]
}
```
`uam_file` - path to UAM file 

`tree_data` - array of objects to describe attribute tree.

`test_data` - array of objects to describe tests.

`node_data_object` - object to describe node in branch:
```json
{
  "branch_descriptor": "0x04020000", 
  "value": {"desired": 0, "reported": 100}}
}
```

`branch_descriptor` - string expression, that describe attribute branch. Examples:

"0x3102[1].0x3015" - attributes as HEX string

"ATTRIBUTE_SENSOR_MULTILEVEL_SENSOR_TYPE[1].ATTRIBUTE_SENSOR_MULTILEVEL_SENSOR_VALUE" - attributes as string

`value` - desired or/and reported value. Set to {} if no value

`test_data_object` - object to describe test as set of scopes with activators and assert rules
```json
{
  "scope": 0, 
  "activator": {node_data_object}, 
  "test_assert": [{node_data_object}]
}
```

`scope` - scope ID from UAM file

`activator` - it means, that rule from UAM file applied after setting last node in branch to some 

desired or reported value 

`test_assert` - array of object to describe assert rules. Assert rules means, that after applying 

of rules all nodes in branch exists and last nodein branch will be set to some desired and/or reported value 