extends Control


var markdown = """öäüß~~strikethrough text~~ literal text *italics text* __fat text__ __*fat italics text*__ [hello](https://github.com/fahrstuhl/godot_markdown/)
soft linebreak.
hard linebreak.  
end of paragraph.

# Table
| foo | bar |
| --- | --- |
| baz | bim |

# yee

- test
- toast

###### tasklist

- [ ] yee
- [x] nee
- [ ] www.autolink.example

hallo welt
paragraph

---

> block quote

paragraph

paragraph

<file:///tmp>

2. hello
2. world
3. 1. even nested lists
   2. are working
3. lists
4. are working

1.  List item one.

	List item one continued with a second paragraph followed by an
	Indented block.

		$ ls *.sh
		$ mv *.sh ~/tmp

	List item continued with a third paragraph.

2.  List item two continued with an open block.

	This paragraph is part of the preceding list item.

	1. This list is nested and does not require explicit item continuation.

	   This paragraph is part of the preceding list item.

~~strikethrough text~~ literal text *italics text* __fat text__ __*fat italics text*__ [hello](https://github.com/fahrstuhl/godot_markdown/)
uiaenrtdüöä"""


func _on_Button_pressed():
	$Label.text = Markdown.convert_markdown(markdown)
