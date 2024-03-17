extends Control


var markdown = """öäüß ~~strikethrough text~~ literal text *italics text* __fat text__ __*fat italics text*__ [hello](https://github.com/fahrstuhl/godot_markdown/)
soft linebreak.
hard linebreak.  
end of paragraph.

# Table
| foo | bar | bims | bums |
| --- | --- | ---  | ---  |
| baz | bim | bengi | bangl |
# yee
- test
- toast

###### tasklist

- [x] yee
- [ ] nee
- [ ] www.autolink.example

hallo welt
paragraph

> block quote

paragraph

	code
	more

paragraph

paragraph

<file:///tmp>

1. hello

2. world
	* even nested lists
	* are working
4. lists

5. are working

6.  List item one.
	List item one `continued` with a second paragraph followed by an
	Indented block.
		$ ls *.sh
		$ mv *.sh ~/tmp
	List item continued with a third paragraph.
7.  List item two continued with an open block.
	This paragraph is part of the preceding list item.
	1. This list is nested and does not require explicit item continuation.
	   This paragraph is part of the preceding list item.
* test
	toast

test

---

~~strikethrough text~~ literal text *italics text* __fat text__ __*fat italics text*__ [hello](https://github.com/fahrstuhl/godot_markdown/)
uiaenrtdüöä"""


func _ready():
	%MarkdownLabel.text = Markdown.convert_markdown(markdown)
