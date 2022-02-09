#!/usr/bin/python3
# this script iterates of the sources file of each crate and checks if the outer
# macro `#![doc(html_no_source)]` is present in the file

import json
import os
import subprocess

cmd = ['cargo', 'metadata', '--no-deps', '--format-version', '1',
       '--manifest-path', os.path.abspath(__file__ + '/../../../Cargo.toml')]
result = subprocess.Popen(cmd, stdout=subprocess.PIPE).stdout.read()
packages = json.loads(result)["packages"]

crate_files = []
for package in packages:
    for target in package["targets"]:
        if "src_path" in target and not target["src_path"].endswith("build.rs") and "/tests/" not in target["src_path"]:
            crate_files.append(target["src_path"])

need_tag = []
for src_f in crate_files:
    with open(src_f) as f:
        if '#![doc(html_no_source)]' not in f.read():
            need_tag.append(src_f)

if need_tag:
    print("the following item(s) leak source code as html in the rust doc:")
    print(*need_tag, sep='\n')
    print(
        "Please add `#![doc(html_no_source)]` to the contents of the file(s)")
    exit(-1)

exit(0)
