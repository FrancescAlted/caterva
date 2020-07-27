import subprocess

subprocess.call('cd ../doxygen && doxygen Doxyfile && cd ../sphinx', shell=True)

project = 'Caterva'
copyright = '2019-2020, Blosc Developers'

import git
repo = git.Repo('./../..')
tags = sorted(repo.tags, key=lambda t: t.commit.committed_datetime)
latest_tag = str(tags[-1])[1:]
release = latest_tag
version = latest_tag

extensions = ["sphinx.ext.mathjax",
              "breathe"]

source_suffix = '.rst'

master_doc = 'index'
language = None

exclude_patterns = ['_build', '**.ipynb_checkpoints', 'Thumbs.db', '.DS_Store']

pygments_style = None
highlight_language = 'none'

html_static_path = ["_static"]
html_theme = 'sphinx_rtd_theme'
html_logo = "_static/caterva.svg"
html_favicon = "_static/caterva-logo.svg"
html_show_sourcelink = False

html_theme_options = {
    "logo_only": True,
}

breathe_projects = {"caterva": "../doxygen/xml/"}
breathe_default_project = "caterva"


def setup(app):
    app.add_css_file('custom.css')
