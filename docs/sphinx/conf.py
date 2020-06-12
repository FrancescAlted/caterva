import subprocess

subprocess.call('cd ../doxygen && doxygen Doxyfile && cd ../sphinx', shell=True)

project = 'Caterva'
copyright = '2020, Blosc Developers'

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

exclude_patterns = ['_build', 'Thumbs.db', '.DS_Store']

pygments_style = None

html_static_path = ["_static"]
html_logo = "_static/caterva.png"
html_favicon = "_static/favicon-caterva.png"
html_show_sourcelink = False

html_theme = "pydata_sphinx_theme"

html_theme_options = {
    "github_url": "https://github.com/Blosc/Caterva",
    "twitter_url": "https://twitter.com/Blosc2",
    "use_edit_page_button": False,
    "show_prev_next": True,
}

html_context = {
    "github_user": "Blosc",
    "github_repo": "cat4py",
    "github_version": "master",
    "doc_path": "doc",
}

breathe_projects = {"caterva": "../doxygen/xml/" }
breathe_default_project = "caterva"


def setup(app):
    app.add_css_file('custom.css')
