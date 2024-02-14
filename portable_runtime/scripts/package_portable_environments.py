###############################################################################
## License
## <b>Copyright 2022  Silicon Laboratories Inc. www.silabs.com</b>
###############################################################################
## The licensor of this software is Silicon Laboratories Inc. Your use of this
## software is governed by the terms of Silicon Labs Master Software License
## Agreement (MSLA) available at
## www.silabs.com/about-us/legal/master-software-license-agreement. This
## software is distributed to you in Source Code format and is governed by the
## sections of the MSLA applicable to Source Code.
##
###############################################################################

import os
import glob
import shutil
import argparse
from pathlib import Path
from urllib.parse import urlparse

portable_runtime_configs = {
    'windows': {
        'binary_path'    : 'x86_64-pc-windows-gnu/$build_type$/unify_portable_cli.exe',
        'resources_path' : 'x86_64-pc-windows-gnu/$build_type$/resources',
        'gui_binary_path': 'x86_64-pc-windows-gnu/$build_type$/unify_portable_gui.exe',
        'gui_additional_files': ['x86_64-pc-windows-gnu/$build_type$/WebView2Loader.dll']
    },
    'macos': {
        'binary_path'    : 'x86_64-apple-darwin/$build_type$/unify_portable_cli',
        'resources_path' : 'x86_64-apple-darwin/$build_type$/resources',
        'gui_binary_path': None,
        'gui_additional_files': []
    },
    'linux': {
        'binary_path'    : 'x86_64-unknown-linux-gnu/$build_type$/unify_portable_cli',
        'resources_path' : 'x86_64-unknown-linux-gnu/$build_type$/resources',
        'gui_binary_path': 'x86_64-unknown-linux-gnu/$build_type$/unify_portable_gui',
        'gui_additional_files': []
    }
}

def arguments_parsing():
    parser = argparse.ArgumentParser(
        description=__doc__)
    parser.add_argument("--target", choices=["windows", "linux", "macos"], required=True)
    parser.add_argument("--build_folder", default=os.path.join(os.path.dirname(__file__), "../target"))
    parser.add_argument("--build_type", choices=["debug", "release"], default="release")
    parser.add_argument("--package", choices=["cli", "gui", "both"], default="both", help="Select package(s) (cli, gui, or both)")

    args = parser.parse_args()
    return args

def package_binary(search_path, to_folder):
    binary_file = glob.glob(search_path)

    assert (
        len(binary_file) == 1
    ), f"Found {len(binary_file)} file(s) fitting the pattern: {search_path}"
    shutil.copy(binary_file[0], to_folder)

def zip_folder(directory: str):
    compressed_file = directory + ".zip "
    try:
        print("Removing any existing Zip package")
        os.remove(compressed_file)
        print(f"{compressed_file} removed successfully")
    except OSError as error:
        pass

    print(f"Compressing {directory} as {compressed_file}")
    status = os.system(f"zip --symlinks -r {compressed_file} {directory}")
    assert status == 0, f"Failed compressing folder {directory}"
    shutil.rmtree(directory)

def tar_folder(directory: str):
    compressed_file = directory + ".tar.bz2 "
    try:
        print("Removing any existing package")
        os.remove(compressed_file)
        print(f"{compressed_file} removed successfully")
    except OSError as error:
        pass

    print(f"Compressing {directory} as {compressed_file}")
    status = os.system(f"tar cjf {compressed_file} --remove-files -C {directory} .")
    assert status == 0, f"Failed compressing folder {directory}"

def package_portable(platform_name, platform_details, package):
    package_path = f"{os.getcwd()}/portable_runtime_{platform_name}"
    shutil.rmtree(package_path, ignore_errors=True) # Remove existing directory to make sure we don't get any errors from existing files during download, extract etc.
    os.makedirs(package_path)

    shutil.copy(os.path.join(os.path.dirname(__file__), "../readme_user_cli.md"), package_path)
    shutil.copy(os.path.join(os.path.dirname(__file__), "../LICENSE.md"),  package_path)

    shutil.copytree(f"{args.build_folder}/{platform_details['resources_path']}".replace("$build_type$", args.build_type), os.path.join(package_path, "resources"), symlinks=True)

    if package in ["cli", "both"]:
        package_binary(f"{args.build_folder}/{platform_details['binary_path']}".replace("$build_type$", args.build_type), package_path)

    if package in ["gui", "both"] and platform_details['gui_binary_path'] is not None:
        package_binary(f"{args.build_folder}/{platform_details['gui_binary_path']}".replace("$build_type$", args.build_type), package_path)
        for additional_file in platform_details['gui_additional_files']:
            package_binary(f"{args.build_folder}/{additional_file}".replace("$build_type$", args.build_type), package_path)

    base = os.path.basename(package_path)
    if platform_name == "windows":
        zip_folder(os.path.basename(package_path))
    else:
        tar_folder(os.path.basename(package_path))

def package_portable_all():
    for os_target in portable_runtime_configs:
        package_portable(os_target, portable_runtime_configs[os_target], args.package)

if __name__ == "__main__":
    global args
    args = arguments_parsing()

    package_portable(args.target, portable_runtime_configs[args.target], args.package)