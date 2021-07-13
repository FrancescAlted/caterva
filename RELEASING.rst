Release procedure
=================

Preliminaries
-------------

- Make sure that you are in `master` branch::

    git checkout master

- Make sure that ``RELEASE_NOTES.rst`` and ``ANNOUNCE.md`` are up to
  date with the latest news in the release.

- Check that *VERSION* symbols in caterva/caterva.h contains the correct info.

- Commit the changes with::

    git commit -a -m "Getting ready for release X.Y.Z"


Testing
-------

Code
++++

Create a new build/ directory, change into it and issue::

  cmake ..
  cmake --build .
  ctest

Documentation
+++++++++++++

Make sure the documentation is rendering well::

  cd docs/sphinx
  make html
  open _build/html/index.html

Tagging
-------

- Create a tag ``X.Y.Z`` from ``master``.  Use the next message::

    git tag -a vX.Y.Z -m "Tagging version X.Y.Z"

- Push the tag to the github repo::

    git push
    git push --tags

- Add the release notes for this tag in the releases tab of github project at:
  https://github.com/Blosc/Caterva/releases


Announcing
----------

- Send an announcement to the blosc list. Use the ``ANNOUNCE.md`` file as skeleton
  (possibly as the definitive version).

- Tweet about this from the @Blosc2 account and rejoice!


Post-release actions
--------------------

- Edit *VERSION* symbols in caterva/caterva.h in master to increment the
  version to the next minor one (i.e. X.Y.Z --> X.Y.(Z+1)-dev).

- Create new headers for adding new features in ``RELEASE_NOTES.md`` and
  add this place-holder instead:

  XXX version-specific blurb XXX

- Commit the changes::

  git commit -a -m "Post X.Y.Z release actions done"
  git push

That's all folks!

