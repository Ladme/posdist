# posdist: Calculate positions of atoms or distances between atoms

`posdist` can calculate positions of selected atoms, distances between atoms, or positions of geometric centers. See below for examples of usage.

## Dependencies

`posdist` requires you to have groan library installed. You can get groan from [here](https://github.com/Ladme/groan). See also the [installation instructions](https://github.com/Ladme/groan#installing) for groan.

## Installation

1) Run `make groan=PATH_TO_GROAN` to create a binary file `posdist` that you can place wherever you want. `PATH_TO_GROAN` is a path to the directory containing groan library (containing `groan.h` and `libgroan.a`).
2) (Optional) Run `make install` to copy the the binary file `posdist` into `${HOME}/.local/bin`.

## Tests

You can validate the installation by running `tests` in the `tests` directory. If you have got `valgrind` installed, it is recommended to run the tests with `valgrind` using `tests/tests VALGRIND`.

## Options

```
Usage: posdist -c GRO_FILE -s SELECTION1 [OPTION]...

OPTIONS
-h               print this message and exit
-c STRING        gro file to read
-f STRING        xtc file to read (optional)
-n STRING        ndx file to read (optional, default: index.ndx)
-s/-a STRING     selection of atoms
-b STRING        another selection of atoms (optional)
-o STRING        output file (default: posdist.dat)
-x/-y/-z         dimension(s) that shall be treated (default: xyz)
-t               calculate properties in time (optional)
-w               calculate center of selection(s) (optional)
-r               treat the selection -b as a reference point (optional)
```

## Usage

Use [groan selection language](https://github.com/Ladme/groan#groan-selection-language) to specify selections of atoms.
Depending on the options supplied, the output of `posdist` is either written into `stdout` or into an output file.

### Using posdist to analyze positions of atoms and groups

**I want to calculate the current position of all selected atoms.**

Specify only one selection using either option `-s` or `-a` and provide just the `gro` file.

Example: `posdist -c md.gro -s "resname POPC"`

**I want to calculate the current center of the selection of atoms.**

Use flag `-w`.

Example: `posdist -c md.gro -s "resname POPC" -w`

**I want to analyze a trajectory and calculate the average positions of all selected atoms.**

Specify only one selection using either option `-s` or `-a` and provide an `xtc` file along with the `gro` file.

Example: `posdist -c md.gro -f md.xtc -s "resname POPC"`

**I want to analyze a trajectory and calculate positions of all selected atoms in time.**

Use flag `-t` and provide both the `gro` and `xtc` files.

Example: `posdist -c md.gro -f md.xtc -s "resname POPC" -t`

**I want to analyze a trajectory and calculate the average position of the center of selection of atoms.**

Use flag `-w` and provide both the `gro` and `xtc` files

Example: `posdist -c md.gro -f md.xtc -s "resname POPC" -w`

**I want to analyze a trajectory and calculate the position of the center of selection of atoms in time.**

Use flags `-w` and `-t` and provide both the `gro` and `xtc` files.

Example: `posdist -c md.gro -f md.xtc -s "resname POPC" -w -t`

**I want to show only the xy-coordinates of the atoms.**

Use flags `-x` and `-y`.

Example: `posdist -c md.gro -f md.xtc -s "resname POPC" -x -y`

Similarly, you can use just the flag `-x` or just the flag `-y` to show only the x- or y-coordinates, respectively.
The same works for the z-coordinate (flag `-z`). All combinations of `-x`, `-y`, and `-z` flags are allowed. These flags can be combined with all the other options.

### Using posdist to analyze distances between atoms and groups

**I want to caculate the current distances between atoms of one selection and atoms of another selection.**

Specify both selections using the options `-a` and `-b` and provide the `gro` file.

Example: `posdist -c md.gro -a "resname POPC" -b "resname SOL"`

**I want to calculate the current distances between all pairs of atoms from one selection.**

Specify the selection twice, using the options `-a` and `-b`.

Example: `posdist -c md.gro -a "resname POPC" -b "resname POPC"`

**I want to calculate the current distance between the centers of one selection and another selection.**

Use flag `-w`.

Example: `posdist -c md.gro -a "resname POPC" -b "resname SOL" -w`

**I want to calculate the current distances between all the atoms of one selection and the center of another selection.**

Use flag `-r`.

Example: `posdist -c md.gro -a "resname SOL" -b "resname POPC" -r`

Distances will be calculated between all the atoms of `resname SOL` and the center of `resname POPC`.

**I want to analyze a trajectory and calculate the average distances between atoms of one selection and atoms of another selection.**

Specify both selections using the options `-a` and `-b` and provide both the `gro` and `xtc` files.

Example: `posdist -c md.gro -f md.xtc -a "resname POPC" -b "resname SOL"`

**I want to analyze a trajectory and calculate the average distance between the centers of one selection and another selection.**

Use flag `-w` and provide both the `gro` and `xtc` files.

Example: `posdist -c md.gro -f md.xtc -a "resname POPC" -b "resname SOL" -w`

**I want to analyze a trajectory and calculate the average distances between all the atoms of one selection and the center of another selection.**

Use flag `-r` and provide both the `gro` and `xtc` files.

Example: `posdist -c md.gro -f md.xtc -a "resname SOL" -b "resname POPC" -r`

**I want to analyze a trajectory and calculate the distances between atoms of one selection and atoms of another selection in time.**

Use flag `-t` and provide both the `gro` and `xtc` files.

Example: `posdist -c md.gro -f md.xtc -a "resname POPC" -b "resname SOL" -t`

**I want to analyze a trajectory and calculate the distance between the centers of one selection and another selection in time.**

Use flags `-w` and `-t` and provide both the `gro` and `xtc` files.

Example: `posdist -c md.gro -f md.xtc -a "resname POPC" -b "resname SOL" -w -t`

**I want to analyze a trajectory and calculate the distances between all the atoms of one selection and the center of another selection in time.**

Use flags `-r` and `-t` and provide both the `gro` and `xtc` files.

Example: `posdist -c md.gro -f md.xtc -a "resname SOL" -b "resname POPC" -r -t`

**I want to only calculate the distance in the xy-plane.**

Use flags `-x` and `-y`.

Example: `posdist -c md.gro -f md.xtc -a "resname POPC" -b "resname SOL" -x -y`

Similarly, you can use just the flag `-x` or just the flag `-y` to calculate the (oriented) distance only in the x- or y-dimensions, respectively.
The same works for the z-coordinate (flag `-z`). All combinations of `-x`, `-y`, and `-z` flags are allowed. These flags can be combined with all the other options.

## Limitations

The program assumes that the simulation box is rectangular and that periodic boundary conditions are applied in all three dimensions.

The program always uses center of _geometry_, instead of center of _mass_.

The program has only been tested on Linux. Probably will not work on anything that is not UNIX-like.
