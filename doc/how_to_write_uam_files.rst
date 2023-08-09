.. meta::
  :description: Guide to UAM files
  :keywords: Mapper, UAM, Mapping

.. raw:: latex

 \newpage

.. _how_to_write_uam_files_section:

How to write UAM files
======================

The attribute mapper is a component that allows to define relations between
attributes stored in the Attribute Store. It can be used for mapping, as
well as easily adding logic around attributes.

Syntax highlighting
-------------------

To make developer easier, we recommend that you enable some syntax highlighting
for the UAM files. A VSCode extension configuration is provided in the mapper
component folder: ``components/uic_attribute_mapper/vscode-extension``.

No binaries are distributed, but can be generated using vsce.
To enable it, navigate to the folder, and generate the package:

.. code-block:: console

  pi@raspberrypi:~/unify $ npm install -g vsce
  pi@raspberrypi:~/unify $ cd ./components/uic_attribute_mapper/vscode-extension
  pi@raspberrypi:~/unify/components/uic_attribute_mapper/vscode-extension $ vsce package
  WARNING  LICENSE.md, LICENSE.txt or LICENSE not found
  Do you want to continue? [y/N] y
  DONE  Packaged: ~/unify/components/uic_attribute_mapper/vscode-extension/attribute-mapper-0.2.0.vsix (7 files, 3.23KB)

Afterwards the ``attribute-mapper-0.2.0.vsix`` file can be installed in VSCode
using the command palette *(Ctrl+Shift+P)*, select ``"Install from VSIX"`` and
provide the path to the generated extension.

Path Configuration
------------------

To enable the Attribute Mapper with user-defined *uam* files, it needs to be
initialized and provided with a directory name containing a set of *.uam*
files.

For example, using an application like the
:doc:`Example Protocol Controller <how_to_develop_a_protocol_controller>`:

.. code-block:: console

  pi@raspberrypi:~ $ ./applications/epc/epc --mapdir /path/to/uam/files/


Attribute Type registration
---------------------------

.. raw:: html

  The mapper allows to manipulate attributes that are stored as numerical values,
  as long as they are registered to the Attribute Store using the
  <a href="../doxygen_uic/group__attribute__store__type__registration.html">
    attribute_store_register_type API
  </a>.

Attribute types not registered to the Attribute Store will be treated as
32-bit signed integer values.

UAM File structure and syntax
-----------------------------

A uam file template could be as follow. It contains a set of definitions and a
scope. The scope itself contains a set of assignments.

.. code-block:: uam

  // this is a comment

  def <name1> expression
  def <name2> expression

  scope 0 <settings> {
    <attribute1> = <expression1>
    <attribute2> = <expression2>
  }


The mapper can identify the following "value states" for each attribute:

* ``r`` for reported value
* ``d`` for desired value
* ``e`` for the existence of the attribute.
  (evaluates to true/false depending on if the attribute exists)

The value states must be used in front of attribute types, using a ``'`` character.
To perform an assignment, the ``=`` sign must be used.
For example, if the reported value of attribute type 1 should take the
desired value of attribute type 2, the following expression can be used:

.. code-block:: uam

  r'1 = d'2

The mapper constantly monitor attribute updates, so whenever
the desired value of attribute 2 is updated, the reported value of attribute
1 will be adjusted.

.. note::

  Assignments to reported values will trigger attribute creations
  in the attribute store.

* ``r'1 = r'2`` will create attribute type 1 and set its reported value
  if it does not exist.

  But
* ``d'1 = r'2`` will not create attribute type 1 and set its desired value
  if it does not exist.

Aliases (def)
'''''''''''''

To avoid using raw type values, the mapper allows to define value substitutions
using the ``def`` keyword. For example, ``r'1 = d'2`` above could be expressed
as:

.. code-block:: uam

  def zwave_state 1
  def zigbee_state 2
  scope 0 {
    r'zwave_state = d'zigbee_state
  }

Operators
'''''''''

Regular operators can be used to modify the value. The available operators are
shown in
:numref:`table_attribute_mapper_available_operators`.

.. list-table:: Unify Attribute Mapper available operators
  :name: table_attribute_mapper_available_operators
  :widths: 20 80
  :header-rows: 1

  * - Operator
    - Name/Description
  * - \+
    - Addition
  * - \-
    - Subtraction
  * - \*
    - Multiplication
  * - /
    - Division
  * - &
    - bitwise AND
  * - \|
    - bitwise OR
  * - %
    - Modulo
  * - \*\*
    - Exponent


For example, if a value needs to be multiplicated by 2 and incremented by 1:

.. code-block:: uam

  def zwave_state 1
  def zigbee_state 2
  scope 0 {
    r'zwave_state = ((d'zigbee_state * 2) + 1)
  }

.. note::

  It is **strongly recommended** to use parentheses around operations, as
  little priority handling is implemented in the mapper.

IF and Comparison Operators
'''''''''''''''''''''''''''

Comparison operators are also available for in UAM maps.
The available comparison operators are shown in
:numref:`table_attribute_mapper_available_comparison_operators`.

.. list-table:: Unify Attribute Mapper available comparison operators
  :name: table_attribute_mapper_available_comparison_operators
  :widths: 20 80
  :header-rows: 1

  * - Operator
    - Name/Description
  * - ==
    - Equal to
  * - !=
    - Not equal to
  * - >
    - Greater than
  * - <
    - Less than
  * - <=
    - Less than or equal to
  * - >=
    - Greater than or equal to

These operators should be used with ``if`` statements. The ``if`` syntax is as
follows: ``if condition value_if_true value_if_false``.
A switch-like statement can be made using multiple ``if`` statements.

.. code-block:: uam

  scope 0 {
    // If the desired value of attribute 2 is 0, set reported of 1 to 1
    // If the reported value of attribute 3 is 3, set reported of 1 to 2
    // if none of the previous conditions is true, set the reported of 1 to 0.
    r'1 =
      if (d'2 == 0) 1
      if (r'3 == 3) 2
      0
  }

Note in the example above that line breaks and indentation does not have any
impact, and are just used for readability. The statement can be written in one
line too. This is an example from the ZPC default maps, which determines if the
reported value of the battery low flag should go to 1 or 0 based on the
reported battery percentage.

.. code-block:: uam

  // [...] Skipping def statements
  r'zbBATTERY_LOW =
    if (r'zwBATTERY < 10) 1
    if (r'zwBATTERY == 0xFF) 1
    0

Note that bitwise operators ``&`` and ``|`` can be used for evaluating a set of
boolean expressions:

.. code-block:: uam

  r'1 =
    if ((d'2 == 0) & (r'3 == 3)) 12
    if (d'2 == 0) 1
    if (r'3 == 3) 2
    0

undefined keyword
'''''''''''''''''

A special ``undefined`` keyword can be used to abort assignment evaluations.
For example, to set the reported value of an attribute **only if** another
attribute exists, the following map can be used:


.. code-block:: uam

  // If attribute type 2 exists, set the value to 1, else do not do anything
  r'1 =
    if (e'2) 1
    undefined

.. note::

  ``undefined`` evaluates to false in an ``if`` condition, so the expression
  ``if (undefined) 1 2`` will evaluate to 2.

An attribute path will evaluate to undefined if either:

1. The attribute type does not exist
2. The value of the attribute is empty, i.e. undefined

or keyword
''''''''''

Additionally, a special ``or`` keyword can be used for fallback values,
in case something is undefined:

.. code-block:: uam

  // If attribute type 2 has a reported value, set the reported value of 1 to this value
  // else set it to the (reported value of attribute type 3) + 2
  r'1 = r'2 or (r'3 + 2)

For example ``undefined or 4`` will evaluate to 4.

Attribute navigation
''''''''''''''''''''

Tree navigation operators can be used to navigate the Attribute Store tree,
for when attributes are organized in a more complicated manner. They are
shown in :numref:`table_attribute_mapper_available_navigation_operators`.

.. list-table:: Unify Attribute Mapper available navigation operators
  :name: table_attribute_mapper_available_navigation_operators
  :widths: 20 80
  :header-rows: 1

  * - Operator
    - Name/Description
  * - []
    - Reported value navigation
  * - .
    - Child navigation
  * - ^
    - Parent navigation

.. warning::

  Parent navigation operator has some known issues and may not work.

.. code-block:: uam

  // If the desired value of (attribute 3 placed under attribute 2 with reported value 1) is greater than 0, set reported of 1 to 1
  r'1 =
    if (d'2[1].3 > 0) 1
    undefined

All attributes are placed under a common parent attribute type, usually
representing an endpoint.
For example, if trying to navigate up the tree, the parent is the endpoint
attribute.

.. code-block:: uam

  def ep 0x00000004
  scope 0 {
    // Set the reported value of attribute 1 to the reported value of attribute 1 under endpoint ID 1 to the same value under endpoint 0.
    r'^.ep[1].1 = r'^.ep[0].1
  }


Built-in functions
''''''''''''''''''

A few built-in functions are available for usage in the UAM files.
Functions are invoked with the following syntax:
``fn_<function_name>(expression, expression, ...)``

The following example shows the use of a function as an assignment:

.. code-block:: uam

  scope 0 {
    // Attribute type 1 will be the absolute value of attribute type 2
    r'1 = fn_absolute_value(r'2)
    // Attribute type 10 will be the min value between the reported value of type 2, 3 or 4
    r'10 = fn_min_value(r'2,r'3,r'4)
  }

The following example shows the use of a function as condition evaluation and assignment.

.. code-block:: uam

  scope 0 {
    r'1 = if (fn_is_any_undefined(r'2, r'3, r'4)) r'6 fn_min_value(r'2, (2 * r'3), (20 % r'3))
  }

.. raw:: html

  The available functions can be found in the
  <a href="../doxygen_uic/group__attribute__mapper__built__in__functions__definitions.html">
    Available Mapper built-in functions
  </a>.
  <br/><br/>

Scope settings
''''''''''''''

Some mapper configuration can be attached to scopes. The available settings
are shown in :numref:`table_attribute_mapper_available_scope_settings`.

.. list-table:: Unify Attribute Mapper available scope settings
  :name: table_attribute_mapper_available_scope_settings
  :widths: 30 20 30 40
  :header-rows: 1

  * - Setting
    - Setting Name
    - Default value
    - Description
  * - :ref:`how_to_write_uam_files_clear_desired_subsection`
    - ``clear_desired``
    - 1
    - Controls if the desired value gets

      undefined automatically during mappings.
  * - :ref:`how_to_write_uam_files_chain_reaction_subsection`
    - ``chain_reaction``
    - 1
    - Controls if the Attribute Mapper triggers new

      evaluations based on attribute

      update it performed itself.
  * - :ref:`how_to_write_uam_files_attribute_creaction_subsection`
    - ``create_attributes``
    - 1
    - Controls when to create new attributes.
  * - :ref:`how_to_write_uam_files_common_parent_type_setting_subsection`
    - ``common_parent_type``
    - Mapper's engine

      Common Parent Type

      configuration
    - Temporarily changes the Common Parent Type for a scope.

The syntax is as follow: ``scope <priority> setting_name(value) setting_name(value) {<assignments>}``
For example, this UAM snippet shows scopes with different configurations.
Note that the priority value has no effect at the moment.

.. code-block:: uam

  scope 0 chain_reaction(0) clear_desired(0) create_attributes(2) {
    r'1 =  r'2
    r'2 =  r'1
    d'1 =  d'2
    d'2 =  d'1
  }
  scope 1 chain_reaction(0) clear_desired(1) {
    r'3 =  d'4
    r'4 =  d'3
  }
  scope 2 chain_reaction(1) clear_desired(0) {
    d'6 =  r'5
    r'6 =  d'6
  }
  scope 3 chain_reaction(1) clear_desired(1) common_parent_type(3) {
    r'8 =  d'9
    r'9 =  r'8
    r'10 = r'9
  }

.. _how_to_write_uam_files_scope_priority:

Scope priority
~~~~~~~~~~~~~~

The Scope priority is used to rank rules by priority, and let overlapping rules
co-exist. Let's take the following example:

.. code-block:: uam

  scope 0 {
    r'2 = r'3
  }
  scope 1 {
    r'2 = r'4
  }

The reported value of Attribute Type 2 is assigned in several maps. Whenever
an attribute of type 3 or 4 is updated, the mapper will try to execute rules
with the highest priority first, and will stop at the first one that return
assigns a value.

* If r'3 is set to 3, then r'2 will be 3.
* If r'3 is set to 3, attribute 4 exists but has no reported, then r'2 will be 3.
* If r'4 is set to 20, then r'2 will be 20 regardless of the value set in r'3.

Here is another scope priority example:

.. code-block:: uam

  scope 10 {
    r'2 = r'3
  }
  scope 100 {
    r'2 = if (r'4>0) r'4 undefined
  }

First, if neither Attribute 3 and 4 exist, then attribute 2 does not exist either.

If r'3 is created and assigned a value, the scope 10 rule will trigger the creation
of r'2. The mapper will verify if the scope 100 rule (which prevails) would lead
to an attribute deletion, and if so, would cancel the scope 10 rule.

Whenever r'4 value is defined and positive, r'2 will be set to the value of r'4.
Else r'2 would fallback on the r'3 value.

For the scope priority to cancel lower priority maps, the mapper checks that
the "destination" attribute to be the same. Taking this map:

.. code-block:: uam

  scope 1 {
    r'3.2 = r'10
  }
  scope 2 {
    r'3[0].2 = r'11
  }

Scope 2 will take precedence only if we have an attribute 2 placed under an
attribute 3 with value 0. In other cases, scope 2 will not affect scope 1.

Note that if 2 equivalent mappings have the same priority, it will be
unpredictable which mapping prevails. If possible, all maps to a given
attribute type should have a unique scope priority.

.. code-block:: uam

  // This is strongly NOT RECOMMENDED:
  scope 1 {
    r'2 = r'3
  }
  scope 1 {
    r'2 = r'4
  }

.. _how_to_write_uam_files_clear_desired_subsection:

Automatic desired value clearance
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

By default, the Attribute Mapper will clear (undefine) the desired value of
an attribute when it sets the reported value.

For example, in the following map:

.. code-block:: uam

  scope 0 {
    d'1 = r'2
    r'1 = r'2
  }

If the reported value of Attribute Type 2 is updated, it will be mapped to both
the desired and reported value of Attribute Type 1, but setting the reported
value of Attribute Type 1 will clear the desired value.

Evaluation steps:

1. r'2 is updated to a new value x
2. d'1 is updated to x
3. d'1 is undefined, r'1 is updated to x.

To prevent this from happening, the scope can be configured not to clear the
desired value.

.. code-block:: uam

  scope 0 clear_desired(0) {
    d'1 = r'2
    r'1 = r'2
  }

Evaluation steps:

1. r'2 is updated to a new value x
2. d'1 is updated to x
3. r'1 is updated to x.

.. _how_to_write_uam_files_chain_reaction_subsection:

Chain reaction evaluations
~~~~~~~~~~~~~~~~~~~~~~~~~~

By default, the Attribute Mapper will react to its own updates, so it can
continously trigger itself to update more values.

Let's take the following map as a example:

.. code-block:: uam

  scope 0 {
    r'2 = r'1
    r'3 = r'2
    r'4 = r'3
  }

Evaluation steps:

1. r'1 is updated to a new value x
2. r'2 is updated to x.
3. r'3 is updated to x.
4. r'4 is updated to x.

When attributes have many dependencies, it can be difficult to keep track of
the chain reaction of changes triggered by one attribute.

.. code-block:: uam

  scope 0 chain_reaction(0) {
    r'2 = r'1
    r'3 = r'2
    r'4 = r'3
  }

Evaluation steps:

1. r'1 is updated to a new value x
2. r'2 is updated to x.

Note that switching off the chain-reaction and desired clearance allows the
mapper to map 2 attributes in all directions (desired <-> reported)
without any infinite loop.

.. code-block:: uam

  scope 0 chain_reaction(0) clear_desired(0) {
    r'1 =  r'2
    r'2 =  r'1
    d'1 =  d'2
    d'2 =  d'1
  }

.. note::

  Chain reaction is always active for attribute creations done automatically
  by regular assignments.

For example, considering the following map:

.. code-block:: uam

  scope 0 chain_reaction(0) {
    r'1 =  r'2
  }
  scope 0 {
    r'3 =  if (e'1) 1 undefined
  }

If the Reported value of Attribute Type 2 is updated and no Attribute
Type 1 exists, then:

1. Attribute Type 1 is created. Other maps depending on the existence of
Attribute Type 1 will be evaluated immediately.
2. Attribute Type 3 is (possibly created), then its reported value is set to 1
3. r'1 is updated to the same value as the reported value of Type 2.

.. _how_to_write_uam_files_attribute_creaction_subsection:

Attribute Creations
~~~~~~~~~~~~~~~~~~~

Scope configurations allows to change the default attribute creation settings.
For example using the following map:

.. code-block:: uam

  scope 0 {
    r'2 =  r'1
    e'3 =  r'1
    d'4 =  r'1
  }

If the Reported value of Attribute Type 1 is updated, the mapper will:

* Create Attribute Type 2 and set its reported value.
* Create Attribute Type 3.
* Not create Attribute Type 4.

A ``create_attributes`` setting can be used to modify this behavior. Here are
the possible values:

* 0. will create attributes only when assigning values types ``e``.
* 1. (default) will create attributes when assigning values types ``e`` and ``r``
* 2. will create attributes when assigning any values type ``e``, ``r`` and ``d``

.. _how_to_write_uam_files_common_parent_type_setting_subsection:

Common Parent Type local override
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The attribute mapper should be configured by default with an Attribute
representing Endpoints, but if mapping needs to happen at another level,
scope settings can locally override the Common Parent Attribute Type using the ``common_parent_type`` scope
configuration.

.. code-block:: uam

  scope 0 common_parent_type(23) {
    // Attribute Type 23 is the "Common Parent Type" for this mapping.
    r'2 =  r'1
    e'3 =  r'1
    d'4 =  r'1
  }

It is recommended to use this setting if mapping have to be done where both
the destination and original attributes are located some levels above or under
the default parent type.

.. code-block:: uam

  def ep 0x00000004
  scope 0 common_parent_type(3) {
    // Set the reported value of attribute 1 to the reported value of
    // attribute 1 under endpoint ID 1 to the same value under endpoint 0.
    // Equivalent to : r'^.ep[1].1 = r'^.ep[0].1
    r'ep[1].1 = r'ep[0].1
  }

Instance assignments
''''''''''''''''''''

Until now, the examples have been using regular assignments. UAM file allows
to add a ``i:``modifier before the left-hand side of an assignment, which
will make it an instance assignemnt. Here is an example:

.. code-block:: uam

  scope 0 {
    i:r'2 = r'3
  }

Instance assignments are intended for making sure that an attribute with a certain
value exists or not. Consider the following map:

.. code-block:: uam

  scope 0 {
    // Create an attribute 2 with reported value 1 if r'3 is greater than 0.
    i:r'2 = if (r'3 > 0) 1 undefined
    // Also create an attribute reported 2 with value 2 if r'3 is greater than 0.
    i:r'2 = if (r'3 > 0) 2 undefined
  }

Here the difference, is that regular assignments would end up creating a single
Attribute 2, and will either set the reported value to 1 or 2.

Instance assignments will create 2 Attributes 2, one with value 1 and one with value 2.
The evaluated values of expression will differ slightly when using instance
assignments. It is recommended to use an index at the end of the left-hand
side to indicate the instance values.

The value type ``e'`` cannot be used with a instance assignment. It must be
either the reported or the desired value.

.. code-block:: uam

  scope 0 {
    // Here the expression will be interpreted as the value that must exist in the reported value of an attribute 2
    i:r'2 = <expression>
    // Here the expression will be interpreted as a boolean expression that indicate if the Attribute 2 with Value 1 should exist or not.
    i:r'2[1] = <expression>
  }

Scope settings with Instance assignments
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Scope priorities are respected within a type of assignment.
There is no guarantee that Instance assignments will run before or after
another type of assignment (regular, clearance), but scope priorities within
the same type of assignement will be respected.

Scope settings will behave as follow with instance assignments:

* ``clear_desired``: will have no effect. The desired_value will not be cleared.
* ``chain_reaction``: works normally
* ``common_parent_type``: works normally
* ``create_attributes``: will have no effect. The instance assignement expressions will dictate if attributes are to be created or deleted.


Clearance assignments
'''''''''''''''''''''

Clearance assignments are run by the mapper before the regular assignments,
if the destination already exists.

Scope priorities are respected within a type of assignment.
There is no guarantee that Clearance assignments will run before or after
another type of assignment (regular, clearance), but scope priorities within
the same type of assignement will be respected.

Until now, the examples have been using regular assignments. UAM file allows
to add a ``c:``modifier before the left-hand side of an assignment, which
will make it an instance assignemnt. Here is an example:

The value type ``e'`` cannot be used with a clearance assignment. It must be
either the reported or the desired value.

.. code-block:: uam

  scope 0 {
    // Clear the reported value of attribute 2 if r'3 != 0.
    c:r'2 = r'3
  }

Right-hand sides in clearance assignments are evaluated like a boolean expression.
If the Right-hand side evaluates to:

* ``0 ``: The value will not be cleared. Stop looking at lower priority clearance maps for the same destination.
* ``!= 0`` : The value will be cleared. Stop looking at lower priority clearance maps for the same destination.
* ``undefined``: Ignore this map and check the lower priority clearance maps for the same destination.

Scope settings with Clearance assignments
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Scope settings will behave as follow with clearance assignments:

* ``clear_desired``: will have no effect.
* ``chain_reaction``: works normally
* ``common_parent_type``: works normally
* ``create_attributes``: will have no effect. Clearance assignements will never create or delete attributes.


Debugging evaluations
'''''''''''''''''''''

When the mapper is running, it can be configured to print its evaluations.
Make sure to run with the debug log level enabled for the ``attribute_mapper``
logging tag.

For debug builds, it will by default print the left-hand side of the assignment,
the Attribute ID that triggered the change and the calculated value.

.. code-block:: console

  <timestamp> <d> [mqtt_client] mqtt_client::on_message: ucl/by-unid/zw-F4DC26FF-000D/ep0/Level/Commands/MoveToLevelWithOnOff - {"Level":85,"TransitionTi..., QoS: 0
  <timestamp> <d> [unify_dotdot_attribute_store_level_commands_callbacks] Updating ZCL desired values after Level:CurrentLevel command
  <timestamp> <d> [unify_dotdot_attribute_store_level_commands_callbacks] Updating ZCL desired values after Level:OnOffTransitionTime command
  <timestamp> <d> [attribute_store_process] Restarting Attribute Store auto-save cooldown timer for 10 seconds.
  <timestamp> <d> [unify_attribute_mapper_process] 1 pending attribute updates to evaluate.
  <timestamp> <d> [attribute_mapper] Checking assignments for Destination ID 2808 (Value), Original Node 2841 (ZCL Level CurrentLevel). (1 candidate(s))
  <timestamp> <d> [attribute_mapper] Match expression: d'( ( Multilevel Switch State ) ).( ( Value ) ) triggered by Attribute ID 2841 (ZCL Level CurrentLevel) affecting Attribute ID 2808 - Result value: 85
  <timestamp> <d> [unify_attribute_mapper_process] Ignoring update to Attribute ID 2808 as the mapper was instructed to ignore it.
  <timestamp> <d> [attribute_mapper] Assigment with priority 0 executed successfully.
  <timestamp> <d> [attribute_mapper] Checking assignments for Destination ID 2841 (ZCL Level CurrentLevel), Original Node 2841 (ZCL Level CurrentLevel). (3 candidate(s))
  <timestamp> <d> [attribute_mapper] Match expression: r'( ( ZCL Level CurrentLevel ) ) triggered by Attribute ID 2841 (ZCL Level CurrentLevel) affecting Attribute ID 2841 - Result value: 50.0824
  <timestamp> <d> [attribute_mapper] Assigment with priority 100 executed successfully.
  <timestamp> <d> [attribute_mapper] Checking assignments for Destination ID 2849 (Value), Original Node 2841 (ZCL Level CurrentLevel). (5 candidate(s))
  <timestamp> <d> [attribute_mapper] Match expression: d'( ( Color Switch State ) ).( ( Component ID ) )[( ( 0x0 ) )].( ( Value ) ) triggered by Attribute ID 2841 (ZCL Level CurrentLevel) affecting Attribute ID 2849 - Result value: 0
  <timestamp> <d> [attribute_mapper] Assigment with priority 100 executed successfully.

If the same attribute ID has several assignments, the full expression can
be logged, by changing ``assignment->lhs`` to ``assignment`` in the
``attribute_mapper_engine.cpp``. Assigning different priority to each assignment
is also a good method to identify which assignment was run.

.. code-block:: cpp

  #ifndef NDEBUG
    // Debug build will print the matched expressions
    std::stringstream ss;
    ss << "Match expression: " << assignment << " triggered by Attribute ID "
      << std::dec << original_node << " ("
      << attribute_store_get_type_name(original_node.type()) << ")"
      << " affecting Attribute ID " << std::dec << destination
      << " - Result value: " << value.value();
    sl_log_debug(LOG_TAG, ss.str().c_str());
  #endif

