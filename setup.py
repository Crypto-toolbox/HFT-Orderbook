from __future__ import annotations

import os
import sys

from Cython.Build import cythonize
from setuptools import Extension, setup


ROOT = os.path.abspath(os.path.dirname(__file__))
SRC_DIR = os.path.join(ROOT, "src")

source_files = [
    "src/datastructs.c",
    "src/limits.c",
    "src/orders.c",
    "src/bst.c",
    "src/utils.c",
    "src/hashmap.c",
    "src/pool.c",
    "src/book.c",
]

compile_args = ["/W4"] if os.name == "nt" and sys.platform.startswith("win") else ["-Wall", "-Wextra"]

ext_modules = [
    Extension(
        "_hftlob",
        ["_hftlob.pyx", *source_files],
        include_dirs=[SRC_DIR, ROOT],
        extra_compile_args=compile_args,
        extra_link_args=[],
        language="c",
    )
]

setup(
    name="hftlob-cython",
    version="0.1.0",
    description="Cython bindings for the HFT order book C library",
    ext_modules=cythonize(
        ext_modules,
        compiler_directives={"language_level": 3},
        include_path=[SRC_DIR, ROOT],
    ),
)
