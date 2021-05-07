import subprocess
import datetime


subprocess.call('cd ../doxygen && doxygen Doxyfile && cd ../sphinx', shell=True)

project = 'Caterva'
author = 'The Blosc Developers'
now = datetime.datetime.now()

copyright = f'2019-{now.year}, {author}'

extensions = ["sphinx.ext.mathjax",
              "breathe",
              "myst_parser"]

source_suffix = ['.rst', '.md']

master_doc = 'index'
language = None

exclude_patterns = ['_build', '**.ipynb_checkpoints', 'Thumbs.db', '.DS_Store']

pygments_style = None
highlight_language = 'none'

html_static_path = ["_static"]
html_theme = "pydata_sphinx_theme"
html_logo = "_static/caterva.svg"
html_favicon = "_static/caterva-logo.svg"
html_show_sourcelink = False

html_theme_options = {
}

html_css_files = [
    'css/custom.css',
]

breathe_projects = {"caterva": "../doxygen/xml/"}
breathe_default_project = "caterva"
