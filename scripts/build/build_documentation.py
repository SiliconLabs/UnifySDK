import subprocess
import os
import argparse

default_project_root = os.path.realpath(
    os.path.join(__file__, "..", "..", ".."))

parser = argparse.ArgumentParser(
    description='Script that build the unify documentation')
parser.add_argument('--doc-dir', default=os.path.join(default_project_root, "doc"),
                    help="directory to where the sphinx conf.py file is located")
parser.add_argument('--build-dir', default=os.path.join(os.getcwd()),
                    help="directory where to export the sphinx documentation to. default=$cwd/sphinx")
parser.add_argument('--builder', choices=['html', 'latex'],
                    default='html', help="which builder to use. default=html")
parser.add_argument('--root_doc', default="doc/introduction",
                    help="which root doc to use. Has to be relative to directory specified in --source. default=doc/contents")
parser.add_argument('--doctree', default=".doctree",
                    help="where to place generated doctree files in the build folder. default=.doctree")
parser.add_argument('--source', default=default_project_root,
                    help=f"directory Sphinx should use as the root of documentation. default={default_project_root}")
args = parser.parse_args()

doc_dir = os.path.realpath(args.doc_dir)
build_dir = os.path.realpath(args.build_dir)
root_doc = args.root_doc # Does not work when using full path, has to be relative to 'documentation_root = args.source'
doctree_dir = os.path.realpath(args.doctree)
documentation_root = args.source
os.environ["UNIFY_BUILD"] = os.path.realpath(args.build_dir)

# run sphinx builder
subprocess.run(
    f"sphinx-build -b {args.builder} -c {doc_dir} -D root_doc={root_doc} -d {doctree_dir} {documentation_root} {build_dir}/sphinx", shell=True)
