# attribute-mapper README

This is a VSCode project for a VSCode extension which provides syntax highligting for 
UAM files. 

All the logic of this takes places in the uam.tmGrammar.json file. There is a launch configuration 
for which may be used when developing this.

To package this into something that can be used by team members do the following:


First you need the vsce node package
```
npm install -g vsce
```

The build the package, in this directory do the following:

```
vsce package
```

This will generate a VSIX file which others can install in VSCode using the vscode command
"Install from VSIX" in the command palette (Ctrl+Shift+P)

Also See [vsc-extension-quickstart.md]()