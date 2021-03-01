# Nepeta

Nepeta is a human-readable file format, with a similar purpose to XML, JSON, and YAML.

What sets it apart is that it has a much simpler syntax, while keeping the most essential
features of the other formats. The format was designed to be simple to understand and modify.

It is written in standard C++17, and can be used as a single embeddable header.

Library documentation is available [here](https://doc.sepiamori.com/nepeta).

This project was originally hosted at <https://git.sepiamori.com/sepia-mori/nepeta>

## Building and installing

Nepeta is intended to be trivially embeddable, so just copying self-contained/nepeta.h into a project should be enough to get started.

For system installation and/or building the documentation, examples, and tests, Nepeta uses CMake for the build system.
Run the below commands to get started (this assumes make is used).

Note that the tests uses the boost unit test framework, so without it installed the tests will not compile.

```
mkdir build
cd build
cmake ..
make
```

To install, run:
```
make install
```

To build the documentation (requires [doxygen](https://www.doxygen.nl) to be installed), run:
```
make doc
```

The documentation can then be opened from build/documentation/html/index.html.

## Format

The underlying data structure is a tree of nodes, where each node has:

- An identifier
- A string array of data
- An array of nested child nodes

The behaviour of the format is inspired by how command-line parameters are passed to programs,
and should be familiar to anyone who has used a command-line.

See documentation/file-format.md for a full outline of the format.

Below is a brief example of how the format looks. Nepeta supports C-style comments:
```cpp
// A single node with the ID 'node_without_children', and two elements in the data array
node_without_children first_data_of_node "second data as string"

// A node with the ID 'node_with_children', a single data element
// and a '#' prefix, meaning subsequent nodes are children of this node.
// An empty '#' marks the end of specifying children for a node
#node_with_children data
	ChildNodeOfNodeWithChildren
#
```

More complex data can be stored in blocks resembling C/C++ blocks. The example below has a single node with multiline data.
```cpp
node {
	Text data spanning multiple lines.
	Whitespace is stripped up to the indentation of the first line.
}
// Equivalent to the C-string:
// "Text data spanning multiple lines.\nWhitespace is stripped up to the indentation of the first line."
```

## Syntax highlighting
Syntax highlighting for the [kate editor](https://kate-editor.org/) can be found under tools/kate-syntax-nepeta.xml.

On linux, copy this file to ~/.local/share/katepart5/syntax/. It should then show up in the menus under Tools > Highlighting > Markup > Nepeta.

## License

Copyright Sepia Mori AS 2021.

The project is distributed under the Boost Software License, Version 1.0.

See the accompanying file LICENSE_1_0.txt, or copy at https://www.boost.org/LICENSE_1_0.txt
