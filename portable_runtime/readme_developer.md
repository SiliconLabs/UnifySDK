# Portable environment

This folder contains simple packaging scripts for downloading Silink and setting up portable runtime environment folders. The script should be executed within ´uic/portable_runtime´.

```bash
portable_runtime$ python3 ../scripts/ci/portable_environment/package_portable_environments.py 
```

This creates portable environment folders and downloads Silink for supported platforms/architectures and creates zip archives ready for deployment.
