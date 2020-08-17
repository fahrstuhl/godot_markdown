extends Control

onready var cmark = preload("res://bin/markdown.gdns").new()

func _on_Button_pressed():
	var markdown = """literal text *italics text* __fat text__ __*fat italics text*__ [hallo](https://welt.what.re)
soft linebreak
hard linebreak  
end of paragraph

---

paragraph

paragraph

<file:///tmp>

2. hallo
2. welt
3. 1. sogar nested listen
   2. funktionieren hoffentlich
3. listen
4. funktionieren

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

	2. List item b.

	This paragraph belongs to item two of the outer list."""
	$Label.bbcode_text = "Data = " + cmark.convert_markdown(markdown)
