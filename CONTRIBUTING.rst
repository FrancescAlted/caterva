Contributing to Caterva
=======================

Caterva is a community maintained project. We want to make contributing to
this project as easy and transparent as possible.


Asking for help
---------------

If you have a question about how to use Caterva, please post your question on
StackOverflow using the `“caterva” tag <https://stackoverflow.com/questions/tagged/caterva>`_.



Bug reports
-----------

We use `GitHub issues <https://github.com/Blosc/Caterva/issues>`_ to track
public bugs. Please ensure your description is clear and has sufficient
instructions to be able to reproduce the issue. The ideal report should
contain the following:

1. Summarize the problem: Include details about your goal, describe expected
and actual results and include any error messages.

2. Describe what you’ve tried: Show what you’ve tried, tell us what you
found and why it didn’t meet your needs.

3. Minimum reproducible example: Share the minimum amount of code needed to
reproduce your issue. You can format the code nicely using markdown::

    ```C
    #import <caterva.h>

    int main() {
        ...
    }
    ```


4. Determine the environment: Indicates the Caterva version and the operating
system the code is running on.

Contributing to code
--------------------

We actively welcome your code contributions. By contributing to Caterva, you
agree that your contributions will be licensed under the `<LICENSE>`_ file of
the project.

Fork the repo
+++++++++++++

Make a fork of the Caterva repository and clone it::

    git clone https://github.com/<your-github-username>/caterva


Create your branch
++++++++++++++++++++

Before you do any new work or submit a pull request, please open an `issue on
GitHub <https://github.com/Blosc/Caterva/issues>`_ to report the bug or
propose the feature you’d like to add.

Then create a new, separate branch for each piece of work you want to do.


Update docstrings
+++++++++++++++++

If you've changed APIs, update the involved docstrings using the `doxygen
format <https://www.doxygen.nl/manual/docblocks.html#cppblock>`_.


Run the test suite
++++++++++++++++++

If you have added code that needs to be tested, add the necessary tests and
verify that all tests pass successfully.
