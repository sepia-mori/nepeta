\page format-examples Examples
\brief Some examples of the file format

[TOC]

# Git config in nepeta
This is a local .git/config converted to nepeta.

```cpp
#core
	repositoryformatversion 0
	filemode true
	bare false
	logallrefupdates true
#

#remote origin
	url "git@git.sepiamori.com:sepia-mori/nepeta"
	fetch "+refs/heads/*:refs/remotes/origin/*"
#

#branch master
	remote origin
	merge "refs/heads/master"
#

#gui
	wmstate normal
	geometry "2560x1314+0+31" 405 212
#
```

# Type description for a game
A type definition of a box that explodes on collision, for a fictional game. The script is written in lua.

```cpp
#OBJECT exploding_box
	Description {
		Box that explodes when it comes into contact with anything.
	}
	
	Model box.obj
	#COLLISION
		Type cube
		Dimensions 10 10 10
	#
	
	Script {
		function create(self)
			self:on_touch(function() self:explode() end())
		end
	}
#
```

# Record of books
A document storing several "lorem ipsum" books.

```cpp
#BOOK "Praesent auctor"
	Author "Aliquam finibus"
	DatePublished 2005-11-05
	Description {
		Sed lorem nibh, fringilla ac dapibus eleifend, lacinia sed leo. Suspendisse tristique ex quis eros placerat,
		nec elementum risus aliquam. Donec commodo molestie turpis. Phasellus quis dui euismod, sodales eros et,
		egestas turpis. Maecenas vulputate quam vitae odio laoreet facilisis. Duis tincidunt ligula at nisl malesuada,
		ut efficitur ligula aliquet. Cras ac vulputate est. Donec congue nunc a nulla dignissim dapibus. Nulla facilisi.
		Vivamus consequat laoreet nisl, a pharetra orci.
	}
#

#BOOK "Nunc a augue faucibus"
	Author "Morbi convallis"
	DatePublished 2012-05-04
	Description {
		Donec tempor maximus sodales. Sed et turpis nulla. Nullam eget ex dolor. Praesent lobortis mattis ipsum ac
		eleifend. Proin consequat mauris nibh, cursus suscipit tellus vehicula id. Nunc sem nunc, pulvinar ac congue a,
		laoreet nec leo.
		
		Curabitur nec finibus augue. Nam cursus libero eget nibh feugiat egestas. Etiam justo nisl,
		dignissim ut sollicitudin eu, pretium ac ante. Quisque maximus arcu mi, sit amet dapibus magna sollicitudin vitae.
		Fusce vitae convallis tellus.
	}
#

#BOOK "Integer imperdiet"
	Author "Curabitur posuere"
	DatePublished 2015-03-11
	Description {
		Suspendisse auctor dolor vel ultricies molestie. Aliquam semper purus ipsum, eget rhoncus dolor molestie sed.
		Vestibulum nibh felis, auctor nec varius sit amet, finibus nec risus. Nunc ligula turpis, viverra eu mi sed,
		tempor consequat dolor. Integer in laoreet risus. Nunc tristique orci sit amet ipsum ultrices, laoreet convallis erat aliquet.
		Nullam porta diam non iaculis congue. Lorem ipsum dolor sit amet, consectetur adipiscing elit.
		
		Morbi a erat id ante rutrum gravida. Morbi at purus auctor, porta metus ac, faucibus ex. Donec ex velit, ultrices in tortor at,
		auctor volutpat arcu. Suspendisse porta nisi nec convallis sollicitudin. Nunc pulvinar non elit et ullamcorper.
		Etiam vitae ultrices tellus. Vestibulum ut odio urna. Maecenas non tortor tortor.
	}
#
```
