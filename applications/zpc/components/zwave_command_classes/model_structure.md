Command Class model is Python object with structure:
```
{
    '_key': '', 
    '_version': '', 
    '_name': '', 
    '_help': '', 
    'cmd': [<cmd_object>],
    'attribute': [<attr_object>]
}
```
cmd_object:
```
{
    '_key': '', 
    '_name': '', 
    '_help': '', 
    '_support_mode': '', 
    'param': [<param_object>]
}
```
param_object:
```
{
    '_key': '', 
    '_name': '', 
    '_type': '', 
    '_version': '', 
    <type_specific_object>
}
```

attr_object:
```
{
    '_name': '', 
    '_version': '', 
    '_parameter': bool, 
    '_parent': '', 
    '_report_func': '', 
    '_get_func': '', 
    '_set_func': '',
    '_key': ''
}
```