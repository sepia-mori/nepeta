\page quickstart Quickstart
\brief Brief overview of how to use the library.

# Using the library
Simply include "nepeta.h". It is recommended to copy the self-contained header directly into the project.

```cpp
#include "nepeta.h"

int main(void)
{
	nepeta::document doc;
	return 0;
}
```

# Creating documents
Documents can be created directly. All variables in nepeta::basic_document are public and can be modified freely.

See nepeta::basic_document for more details.

```cpp
// Documents can be initialized directly.
const nepeta::document doc_direct {
	// Identifier: doc_direct.id=
	"",
	// Data: doc_direct.data=
	{},
	// Children: doc_direct.children=
	{
		{"child1", {"data_for_child1"}},
		{"child2", {"data_for_child2"}}
	}
};

// Alternate way to initialize a document
nepeta::document doc_indirect;
doc_indirect.children.push_back({"child1"});
doc_indirect.children.back().data.emplace_back("data_for_child1");
doc_indirect.children.push_back({"child2"});
doc_indirect.children.back().data.emplace_back("data_for_child2");
```

# Parsing documents
Documents can be parsed from strings, typically using either nepeta::load or nepeta::load_view.

See \ref parsing for more ways to load documents from strings.

For details on the syntax of the file format, see the \ref format.

```cpp
const std::string_view source = R"NYA(
	child1 data_for_child1
	child2 data_for_child2
)NYA";

// Load the document from 'source'
const nepeta::document doc_direct = nepeta::load(source);

// Or as a document view, be careful to keep string_source in scope for as long as doc_view exists.
std::string string_source = std::string(source);
const nepeta::document_view doc_view = nepeta::load_view(string_source);
```

# Handling parser errors
To handle parser errors, pass a lambda or function object with compatible parameters to the parsing function.

The function object must satisfy the error handler signature, shown below.

```cpp
// Signature for the error handler.
auto error_handler = [](nepeta::parser_error type, char character, std::size_t line, std::size_t column)
{
	// Do something with the error
};

const std::string_view source = R"NYA(
	child1 data_for_child1
	child2 data_for_child2
)NYA";

const nepeta::document doc = nepeta::load(source, error_handler);
```

# Finding nodes
There are various ways to find child nodes in a document.

The below example shows multiple ways to find children.

See \ref algorithm for more details and ways to interact with documents, or \ref howto for further examples.

```cpp
// Assuming we have this document
const nepeta::document doc;

// The child nodes of a document can be indexed directly
const nepeta::document& child = doc.children.at(0);

// Find the first and last child with the identifier 'child'
// If no such identifier was found, returns nullptr
const nepeta::document* first_child = nepeta::find(doc, "child");
const nepeta::document* last_child = nepeta::rfind(doc, "child");

// Iterate through all children with a given identifier
nepeta::for_each(doc, "child", [](const nepeta::document& doc)
{
	// ...
});
```

# Using data
The data stored within any node can be indexed directly using 'data'.

There are some functions to convert data to integers and booleans. See \ref algorithm for more details.

```cpp
// Assuming we have this child document
const nepeta::document doc;

// Index 'data' directly. 'data' is a normal vector of type std::vector<std::string>.
const std::string& first_data = doc.data.at(0);

// Get an optional string view if the data doesn't exist.
const std::optional<std::string_view> first_data_view = nepeta::opt_data(doc, 0);

// Cast to an integer, returning std::nullopt if casting fails
const std::optional<int> first_value = nepeta::doc_opt_integer<int>(doc, 0);

// Cast to an integer, returning a default value if it fails.
const int first_value_default = nepeta::doc_as_integer<int>(doc, 0, 0);
```

# Writing documents
Documents and document views can be written to strings or streams.

The writer can be passed extra parameters to control how the documents are written, see nepeta::writer_parameters and \ref writer for more details.

```cpp
// Assuming we have this document
const nepeta::document doc;

// Writing to a string
const std::string string_representation = nepeta::write_to_string(doc);

// Writing to a stream
nepeta::write_to_stream(std::cout, doc);
```
