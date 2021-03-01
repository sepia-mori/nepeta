\page format Nepeta file format
\brief Description of the nepeta file format

The syntax is similar to a typical command line, with some changes. Executable names are the equivalent of identifiers for nodes, and the parameters are the data.

See the [reference](#reference) at the bottom for terminology.

[TOC]

# Overview {#overview}

Below is a commented example of a nepeta document, using the features described in this document.

```cpp
// Lines creating nodes without any data
node_identifier
"string-type node identifier"

// Lines creating nodes with two data each
node_with_two_data data_1 data_2
"node with two data" data_1 data_2

// Using string-type data, with escape codes
node_with_string_data "stringed data" "newline\nstring"

// Special character handling (aborting data context, or continuing data on the next line)
first_node ; second_node
node \
	data_for_node

// Various ways to create block data
node_with_multiple_block_data {
	Block data
	across multiple lines
} {
	\ Block data starting with whitespace
} {base64
	QmFzZTY0IGJs
	b2NrIGRhdGE=
}

// Creating nested nodes in a hierarchy
#parent_node data_for_parent_node
	nested_node data_for_nested_node
	
	#deeper_nesting
		child_node_of_deeper_nesting
	#
#
```

# Node parsing {#node}

Node parsing parses child nodes into a parent node (or the root document). They are created by having either an [identifier-](#identifier) or [string-type data](#string) data on a line.

After the new node has an identifier, the parses switches to data parsing for the new node.

Below is an example document demonstrating the two ways of creating nodes.

```cpp
plain_node
"stringed node"
```

This creates a document with two child nodes, the nodes have the identifiers "plain_node" and "stringed node" respectively.

# Data parsing {#data}
When a node identifier has been read, data parsing starts.

Data for the node is parsed until the end of the line.
Each data is separated by whitespace, and can be either identifiers, strings, or blocks. Other than how
data is parsed, there is no distinction between them once they are added to a node.

## Identifier data {#identifier}
Identifiers are a sequence of printable characters, excluding whitespace, binary, and special characters.

See the reference for [valid identifier characters](#identifier-characters) for more details.

Below is an example of a document that creates a single node having three identifier-type data.

```cpp
node first_data second_data third_data
```

## String data {#string}
Strings start with a quote marker ", and ends when the next " is reached. Any \\ and " within the string must be escaped.

See the reference for [valid escape sequences](#escape-sequences) for more details.

Below is an example of a document that creates a single node having three string-type data, including the use of escaped characters.
```cpp
node "first data" "second\\data" "third\"data"
```

## Block data {#block}
Blocks can run over multiple lines. They start with '{', and run until the line where '}' is the first character.

Blocks are the only data type that is whitespace sensitive. The amount of whitespace up to the first line is skipped for every subsequent line,
where spaces and tabs both count as a single character. A newline character is added for every line in the block, except the last.

Apart from the characters that need to be escaped, all characters are valid within a block, including binary data.

Below are a few examples of blocks and their corresponding C-strings:
```c
node {
	Block spanning
	multiple lines
}
// Equivalent to the C-string: "Block spanning\nmultiple lines"

node {
	Block spanning
		multiple lines
}
// Equivalent to the C-string: "Block spanning\n\tmultiple lines"

node {
	Block spanning
	multiple lines
	
}
// Equivalent to the C-string: "Block spanning\nmultiple lines\n"
```

The characters \\ and \\r (carriage-return) must always be escaped within blocks. \\ must be escaped to allow escape characters at all,
and \\r is necessary to make cross-platform newline handling work properly. Normally carriage returns are considered newline characters.
Below is an example of these characters being escaped.

```c
node {
	Backslash: \\, Carriage-return: \r
}
// Equivalent to the C-string: "Backslash: \\, Carriage-return: \r"
```

If the very first character of a block is supposed to be whitespace, it must be escaped as well. See the example below:

```c
node {
	\ the first character is whitespace
}
// Equivalent to the C-string: " the first character is whitespace"
```

## Base64 block {#base64}
Blocks can also store binary-data formatted as base64. Blocks can normally store any binary data as long
as important characters are properly escaped, meaning that support for base64 is primarily for text-editor convenience.

Base64 encoding can be specified on the same line as the opening '\{', like below:
```cpp
node { base64
	SGVsbG8sIHdvcmxkIQ==
}
// Equivalent to the C-string: "Hello, world!"
```

Newlines are intentionally ignored inside base64 data, so the base64 data can be distributed across multiple lines, as below.
```cpp
node { base64
	SGVsbG8sIH
	dvcmxkIQ==
}
// Equivalent to the C-string: "Hello, world!"
```

## Special characters

### End of data parsing ;

; stops the data parsing context for the node, and moves back to node parsing.
```cpp
node_a data_for_a ; node_b data_for_b
```

The above string creates two child nodes for the root document.
The first node has the identifier "node_a", and a single data element "data_for_a".
The second node has the identifier "node_b", and a single data elements "data_for_b".

### Continue on the next line \\
\\ continues data parsing on the next line.
```cpp
node_a data_1_for_a \
	data_2_for_a
```

The above string creates a single child node for the root document, which has two data elements: "data_1_for_a", and "data_2_for_a".

# Nested nodes
Having \# in front of the node identifier allows child nodes to be added to the node. Adding child nodes lasts until an empty \# is reached.

Data can still be added to the parent nodes as normal. The only difference is that new nodes are added to this node instead of the root node.

Below are examples of how to create nested nodes.

```c
// Creating nested nodes in a hierarchy
#parent_node data_for_parent_node
	nested_node data_for_nested_node
	
	#deeper_nesting
		child_node_of_deeper_nesting
	#
#
// parent_node has nested_node and deeper_nesting as children.
// deeper_nesting has child_node_of_deeper_nesting as a child.
```

# Comments
Comments can be added in any context, except inside a data block.

Single-line comments start with '//', and runs until the end of the line.

Multi-line comments start with '/*', and end whenever the first '*/' is reached. Nesting multi-line comments is not allowed.

Below are some comment examples:
```cpp
// This is a single line comment
this_is_a_node
/*
	This is a multiline comment
	this_is_not_a_node
*/

this_is_a_node /*this_data_is_commented_out*/ data

node {
	/*this is not a comment*/
}
```

# Reference {#reference}

## Terminology {#terminology}

Term            | Description
----------------|-------------
document		| A document is a nested tree structure named with an identifier, and storing data and other child documents. See \ref nepeta::basic_document.
root document	| The root document is the top-level document of a tree. The root document doesn't have an identifier or data.
node			| A node is another name for a document, typically used when refering to a specific document within a document tree.
ID/identifier	| The name associated with a document. It can be empty, and does not have to be unique.
data			| An array of plain strings stored inside a document.
child node		| A child node is a document stored as a child of another document.

## Valid identifier characters {#identifier-characters}

This is a list of all characters that are valid as identifiers.

Essentially all characters are valid, except for control characters (ASCII <32), whitespace, newlines, and the special characters: #"{};\\

Type                                   | Description
---------------------------------------|----------------
a-zA-Z0-9                              | All alphanumeric characters.
\!$%&'\(\)\*\+,\-\./:<=>?@\[\]^\_\`\|~ | Other printable characters.
(127 or \\x7F)                         | The delete character.
(unicode)                              | All unicode characters above traditional ASCII are valid. Essentially all characters >127.

## Valid escape sequences {#escape-sequences}

This is a list of valid escape sequences, and their corresponding character representation.

Note that most of these correspond to the same escape sequences as in C/C++. In some cases, the character is simply escaping the corresponding normal character.

Type         | Description
-------------|----------------
\\0          | "Null character", ASCII character 0x00.
\\a          | Audible bell, ASCII character 0x07.
\\b          | Backspace, ASCII character 0x08.
\\f          | Form feed, ASCII character 0x0c.
\\n          | Line feed/newline, ASCII character 0x0a.
\\r          | Carriage return, ASCII character 0x0d.
\\t          | Horizontal tab, ASCII character 0x09.
\\v          | Vertical tab, ASCII character 0x0b.
\\'          | Single quote: '
\\\"         | Double quote: "
\\\\         | Backslash: \\
"\\ "        | Space character (' '), ASCII character 0x20.
"\\	"        | Horizontal tab ('\\t'), ASCII character 0x09.
\\{          | Opening brace: {
\\}          | Closing brace: }

