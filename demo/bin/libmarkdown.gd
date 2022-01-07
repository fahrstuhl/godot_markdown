extends Node
class_name Markdown

static func convert_markdown(text):
	var cmark = Cmark.new()
	var bbcode_text = cmark.convert_markdown(text)
	print(bbcode_text)
	for heading in ["heading_1", "heading_2", "heading_3", "heading_4", "heading_5", "heading_6"]:
		var replace_open = "[font=res://fonts/libmarkdown/{0}.tres]".format([heading])
		var replace_close = "[/font]"
		var tag_open = "[{0}]".format([heading])
		var _tag_close = "[/{0}]".format([heading])
		var _test = tag_open.format({heading: replace_open}, "[_]")
		bbcode_text = bbcode_text.format({heading: replace_open}, "[_]")
		bbcode_text = bbcode_text.format({heading: replace_close}, "[/_]")
	return bbcode_text
