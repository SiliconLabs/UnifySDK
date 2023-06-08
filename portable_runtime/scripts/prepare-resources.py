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
from urllib.parse import urlparse
import tempfile
import hashlib
from pathlib import Path
import requests

platform_tools = {
    'linux' : {
        'target_folder'   : 'x86_64-unknown-linux-gnu/$build_type$/resources',
        'resources'       : [
            {
                'download_name'   : 'Silink',
                'download_url'    : 'https://www.dropbox.com/s/raw/9bmz6gkzw8rvsbx/silink_linux_0v5p2b238.zip',
                'download_sha256' : 'beefd5a112fc3ede3b136fc504c57f8b42c0bda17361fdb5cb12f27bca136516',
                'extract_details' : [
                    {
                        'extract_src' : 'linux_x86_64',
                        'extract_dst' : 'silink',
                    }
                ]
            },
            {
                'download_name'   : 'Commander',
                'download_url'    : 'https://www.dropbox.com/s/raw/0ymemwe0n2j9uxd/Commander_linux_x86_64_1v13p3b1203.tar.bz',
                'download_sha256' : 'e6bda7f995e7e313d16a724d15cf679fc4c69e469dbb894496be9de5f0ce5c7f',
                'extract_details' : [
                    {
                        'extract_src' : 'commander',
                        'extract_dst' : 'commander',
                    }
                ]
            }
        ]
    },
    'macos' : {
        'target_folder'   : 'x86_64-apple-darwin/$build_type$/resources',
        'resources'       : [
            {
                'download_name'   : 'Silink',
                'download_url'    : 'https://www.dropbox.com/s/raw/pfuyjbbhzjqbiwf/silink_osx_0v5p2b238.zip',
                'download_sha256' : 'f7fae2ecd4567b30e9631a6b2431e8a3435e05e330ceb4655cf9ad9f6a73227a',
                'extract_details' : [
                    {
                        'extract_src' : 'macosx',
                        'extract_dst' : 'silink',
                    }
                ]
            }
        ]
    },
    'windows' : {
        'target_folder'   : 'x86_64-pc-windows-gnu/$build_type$/resources',
        'resources'       : [
            {

                'download_name'   : 'Silink',
                'download_url'    : 'https://www.dropbox.com/s/raw/y68fq3uvncak8xa/silink_win_0v5p2b238.zip',
                'download_sha256' : 'a3a32839bb2a9a9932a22b8a1bc144eacdd7f6dbf442aa62c1f240630fddc9c0',
                'extract_details' : [
                    {
                        'extract_src' : 'win32',
                        'extract_dst' : 'silink',
                    }
                ]
            },
            {
                'download_name'   : 'Commander',
                'download_url'    : 'https://www.dropbox.com/s/raw/swansrcncfc16r2/Commander_win32_x64_1v13p3b1203.zip',
                'download_sha256' : '790ea39d172466817654ebdb9ba6c88be8ab1028eff9ab03f61b7886e8dfc2a1',
                'extract_details' : [
                    {
                        'extract_src' : 'Simplicity Commander',
                        'extract_dst' : 'commander',
                    }
                ]
            }
        ]
    }
}

common_resources = [
    {
        'download_name'   : 'Application Firmware',
        'download_url'    : 'https://github.com/SiliconLabs/gecko_sdk/releases/download/v4.2.2/demo-applications.zip',
        'download_sha256' : '229b49a73c43cff7749a26a77487b31d4cef59060d75322b229a6a50f07ba188',
        'extract_details' : [
            {
                'extract_src' : 'protocol/z-wave/demos/zwave_soc_switch_on_off/zwave_soc_switch_on_off-brd2603a-eu.hex',
                'extract_dst' : 'app-binaries/zwave_soc_switch_on_off-brd2603a-eu.hex',
            },
            {
                'extract_src' : 'protocol/z-wave/demos/zwave_soc_multilevel_sensor/zwave_soc_multilevel_sensor-brd2603a-eu.hex',
                'extract_dst' : 'app-binaries/zwave_soc_multilevel_sensor-brd2603a-eu.hex',
            },
            {
                'extract_src' : 'protocol/z-wave/demos/zwave_ncp_serial_api_controller/zwave_ncp_serial_api_controller-brd2603a-eu.hex',
                'extract_dst' : 'app-binaries/zwave_ncp_serial_api_controller-brd2603a-eu.hex',
            }
        ]
    },
    {
        'download_name'   : 'Unify Debian packages',
        'download_url'    : 'https://github.com/SiliconLabs/UnifySDK/releases/download/ver_1.3.1/unify_1.3.1_x86_64.zip',
        'download_sha256' : '21798654079347c79e30735a904dc293e64b0ab691a8fccd9fc660f69364af5e',
        'extract_details' : [
            {
                'extract_dst' : 'docker-files/',
            }
        ],
    }
]

def download_file(download_folder, link):
    file_name = link.split('/')[-1]
    file_path = f"{download_folder}/{file_name}"
    r = requests.get(link, stream = True)
    with open(file_path, 'wb') as f:
        for chunk in r.iter_content(chunk_size = 1024*1024):
            if chunk:
                f.write(chunk)

    return file_path

def extract_file_to_folder(compressed_file, to_folder):
    print(f"Extracting {compressed_file} to {to_folder}")

    os.system(f"rm -rf {to_folder} && mkdir -p {to_folder}")

    if compressed_file.endswith('.zip') :
        status = os.system(f"unzip -q {compressed_file} -d {to_folder}")
        assert status == 0, f"Failed extracting {compressed_file}"
    else :
        status = os.system(f"tar xf {compressed_file} -C {to_folder}")
        assert status == 0, f"Failed extracting {compressed_file}"

    print(f"Extraction of {compressed_file} done!")

def package_binary(search_path, to_folder):
    binary_file = glob.glob(search_path)

    assert (
        len(binary_file) == 1
    ), f"Found {len(binary_file)} file(s) fitting the pattern: {search_path}"
    shutil.copy(binary_file[0], to_folder)

def zip_folder(directory: str):
    zip_file = directory + ".zip "
    try:
        print("Removing any existing Zip package")
        os.remove(zip_file)
        print(zip_file + " removed successfully")
    except OSError as error:
        pass

    print(f"Zipping {directory} as {directory}.zip")
    status = os.system("zip --symlinks -r " + directory + ".zip " + directory)
    assert status == 0, f"Failed zipping folder {directory}"
    shutil.rmtree(directory)

def verify_hash(file_path, known_hash):
    print(f"Verifying hash for file: {file_path}")
    print(f"Known hash:      {known_hash}")
    sha256_hash = hashlib.sha256()
    with open(file_path, "rb") as f:
        # Read and update hash string value in blocks of 4K
        for byte_block in iter(lambda: f.read(4096),b""):
            sha256_hash.update(byte_block)

        print(f"Calculated hash: {sha256_hash.hexdigest()}")
        assert (sha256_hash.hexdigest() == known_hash)

def get_resources(resource, target_folder):
    with tempfile.TemporaryDirectory() as download_folder_path:
        print(f"Downloading {resource['download_name']} from {resource['download_url']}")
        file_path = download_file(download_folder_path, resource['download_url'])

        print(f"Downloaded file path: {file_path}")
        verify_hash(file_path, resource['download_sha256'])

        with tempfile.TemporaryDirectory() as extract_folder:
            extract_srcs = list(filter(lambda e: 'extract_src' in e, resource['extract_details']))

            if len(extract_srcs) > 0:
                extract_file_to_folder(file_path, extract_folder)

            for extract in resource['extract_details']:
                move_from = f"{extract_folder}/{extract['extract_src']}" if 'extract_src' in extract else file_path
                move_to = f"{target_folder}/{extract['extract_dst']}"

                move_to_path = Path(move_to)

                move_from_path = Path(move_from)
                print(f"Copying {move_from} to {move_to}")
                if os.path.isdir(move_from_path):
                    shutil.rmtree(move_to_path, ignore_errors=True)
                    shutil.copytree(move_from_path, move_to_path, symlinks=True)
                else:
                    os.makedirs(move_to_path.parent.absolute(), exist_ok=True)
                    shutil.copy(move_from_path, move_to_path)

def arguments_parsing():
    parser = argparse.ArgumentParser(
        description=__doc__)
    parser.add_argument("--target", choices=["windows", "linux", "macos"], required=True)
    parser.add_argument("--build_folder", default=os.path.join(os.path.dirname(__file__), "../target"))
    parser.add_argument("--build_type", choices=["debug", "release"], default="release")
    parser.add_argument("--unify_path", type=str, required=False)
    args = parser.parse_args()
    return args

if __name__ == "__main__":
    global args
    args = arguments_parsing()

    resource_folder = os.path.join(os.path.dirname(__file__), "../resources")

    platform_resources = platform_tools[args.target]

    copy_dst = os.path.join(args.build_folder, platform_resources['target_folder'])
    copy_dst = f"{copy_dst}".replace("$build_type$", args.build_type)

    shutil.rmtree(copy_dst, ignore_errors=True)
    shutil.copytree(resource_folder, copy_dst, symlinks=True)

    for resource in platform_resources['resources']:
        get_resources(resource, copy_dst)
        print()

    for resource in common_resources:
        if resource['download_name'] == 'Unify Debian packages' and args.unify_path is not None:
            # Copy the unifySDK to all the destinations mentioned in common_resources
            for extract in resource["extract_details"]:
                shutil.copy(args.unify_path, copy_dst + "/" + extract["extract_dst"])
        else:
            get_resources(resource, copy_dst)
        print()
