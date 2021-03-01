\page howto How-to
\brief Code snippets demonstrating how to do common tasks

See also example/basic_usage.cpp for a full code example.

# Iterating through child nodes
Iterating through child nodes can be done either by indexing 'children' directly, or using any \ref algorithm for finding nodes.

```cpp
const nepeta::document doc;

// Directly indexing 'children'
for(const nepeta::document& node : doc.children)
{
	if(node.id == "node")
	{
		// Operate on the node
	}
}

// Using nepeta::begin / nepeta::end / nepeta::next
for(auto i = nepeta::begin(doc, "node"); i != nepeta::end(doc); i = nepeta::next(doc, i, "node"))
{
	const nepeta::document& node = *i;
	// Operate on the node
}
	
// Using nepeta::for_each
nepeta::for_each(doc, "node", [](const nepeta::document& node)
{
	// Operate on the node
});
```

# Reverse iteration of child nodes
All algorithms for finding and iterating child nodes have a reverse variant prefixed with 'r'.

See nepeta::rfor_each, nepeta::rfind, nepeta::rbegin, nepeta::rend, and nepeta::rnext.

# Newline at the end of a block
In order to have an extra newline at the end of a block, simply add an empty line.

```cpp
Node {
	Block without newline
}
// C-string: "Block without newline"

Node {
	Block with a single newline
	
}
// C-string: "Block with a single newline\n"
```
