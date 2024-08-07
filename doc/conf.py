import sphinx.application
from pygments.lexers.data import JsonLexer
from pygments.lexer import using
import os

# UAM lexer, to be moved in a separate file if possible.
from pygments.lexer import RegexLexer
from pygments.token import *


class uam_lexer(RegexLexer):
    name = 'UAM'
    aliases = ['uam']
    filenames = ['*.uam']

    tokens = {
        'root': [
            (r'//.*?$', Comment.Singleline),
            (r"scope|def", Name.Builtin),
            (r"if", Name.Builtin),
            (r"[rde]'[a-zA-Z0-9_]+(\[[a-zA-Z0-9_]+\])*(\.[a-zA-Z0-9_]+(\[[a-zA-Z0-9_]+\])*)*", Name.Variable),
            (r"[a-zA-Z0-9_]+", Name.Constant),
            (r'0x[0-9A-Fa-f]+', Token.Literal),
            (r'[0-9]+.[0-9]+f', Token.Literal),
            (r'[0-9]+', Token.Literal),
            (r"/|<|>|&|or|%|-|==", Operator.Word),
            (r"\*\*?", Operator.Word),
            (r"=", Operator),
            (r"\+", Operator.Word),
            (r"\|", Operator.Word),
            (r"i:", Generic.Emph),
            (r"c:", Generic.Emph),
            (r"undefined", Name.Variable.Magic),
            (r"chain_reaction", Generic.Heading),
            (r"clear_desired", Generic.Heading),
            (r"create_attributes", Generic.Heading),
            (r"common_parent_type", Generic.Heading),
            (r"fn_[a-zA-Z0-9_]+", Name.Function),
            (r'\s+', Text),
            (r'.', Text),
        ]
    }


# MQTT Lexer
class mqtt_lexer(RegexLexer):
    name = 'MQTT'
    aliases = ['mqtt']
    filenames = ['*.mqtt']

    tokens = {
        'root': [
            (r'//.*?$', Comment.Singleline),
            (r'#.*?$', Comment.Singleline),
            (r"^[a-zA-Z0-9_/\-<>+#]+", Operator.Word),
            (r'\{', Text, 'json-object'),
            (r'.', Text),
        ],
        'json-object': [
            (r'\}', Text, '#pop'),
            (r'\{', Text, '#push'),
            (r'[^\{\}]*|\n', using(JsonLexer)),
        ]
    }


# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = "Unify"
copyright = "Copyright © 2023 Silicon Laboratories. All rights reserved."
author = "Silicon Labs"
language = "en"

root_doc = "doc/introduction"

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration
html_additional_pages = {'index': 'index.html'}

extensions = [
    "breathe",
    "myst_parser",
    "sphinxcontrib.plantuml",
    "sphinx.ext.githubpages",
    "sphinx_markdown_tables"
]

templates_path = ["_templates"]
exclude_patterns = ["_build",
                    "Thumbs.db",
                    ".DS_Store",
                    "**internal.md", "**/libs/cmock",
                    "components/testframework",
                    "**zwave_command_classes/README.md",
                    "**vscode-extension**",
                    "**dmt_match_generator**",
                    "**zw-libs2**",
                    "**uic-resources**",
                    "**dotdot-xml**",
                    "build**",
                    "LICENSE.md",
                    "README.md",
                    "**node_modules**",
                    "**uic_attribute_mapper/src/README.md",
                    "scripts/ci",
                    "scripts/sd_card",
                    "**release_notes.md",
                    "externals",
                    "**build_**",
                    "unify-spec",
                    "scripts/ci",
                    "**applications/uhab**",
                    "**cc_template.**"]

source_suffix = {
    ".rst": "restructuredtext",
    ".md": "markdown",
}

# enable all markdown features of the myst parser
myst_enable_extensions = [
    "amsmath",
    "colon_fence",
    "deflist",
    "dollarmath",
    "fieldlist",
    "html_admonition",
    "html_image",
    "linkify",
    "replacements",
    "smartquotes",
    "strikethrough",
    "substitution",
    "tasklist",
]

# setup plantuml plugin
plantuml = "java -jar " + os.environ["PLANTUML_JAR_PATH"]


# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output
html_theme = "sphinx_rtd_theme"
html_static_path = ["_static"]
html_js_files = ["js/charenc.js", "js/crypt.js", "js/sha1.js", "js/html5-qrcode.min.js", "js/qr_code_scanner.js"]
html_css_files = ["custom_styles.css"]
html_logo = "doxygen/assets/silicon-labs-logo.png"
html_title = "Unify Host SDK"
html_baseurl = "https://siliconlabs.github.io/UnifySDK"

releases_github_path = "SiliconLabs/UnifySDK"

# -- Options for figures and tables (https://www.sphinx-doc.org/en/master/usage/configuration.html)
numfig = True
numfig_format = dict()
numfig_format['figure'] = 'Figure %s'
numfig_format['table'] = 'Table %s'
numfig_format['code-block'] = 'Listing %s'
numfig_format['section'] = 'Section %s'

# -- Options for LaTeX output ------------------------------------------------
latex_engine = "lualatex"

latex_elements = {
    # The paper size ('letterpaper' or 'a4paper').
    # I just also aded "oneside", this string is just copied into the \documentclass[] content
    #
    "papersize": "a4paper,oneside",
    # The font size ('10pt', '11pt' or '12pt').
    #
    "pointsize": "10pt",
    # Latex figure (float) alignment
    #
    "figure_align": "H",
}

# Grouping the document tree into LaTeX files. List of tuples
# (source start file, target name, title,
#  author, documentclass [howto, manual, or own class]).
latex_documents = [
    (
        root_doc,
        "unify_sdk_documentation.tex",
        "Unify Framework documentation",
        "Silicon Labs",
        "howto",
    ),
    (
        "doc/unify_specifications/index",
        "unify_specifications.tex",
        "Unify Specifications",
        "Silicon Labs",
        "howto",
    ),
]

# Logo
latex_logo = "./doxygen/assets/silicon-labs-logo.png"

def setup(app: sphinx.application.Sphinx):
    app.add_lexer('uam', uam_lexer)
    app.add_lexer('mqtt', mqtt_lexer)
