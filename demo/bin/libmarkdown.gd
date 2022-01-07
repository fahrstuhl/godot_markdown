extends Node
class_name Markdown

static func convert_markdown(text):
	var heading_sizes = {
		"heading_1" = 32,
		"heading_2" = 28,
		"heading_3" = 24,
		"heading_4" = 20,
		"heading_5" = 18,
		"heading_6" = 16
	}
	var cmark = Cmark.new()
	var bbcode_text = cmark.convert_markdown(text)
	print(bbcode_text)
	bbcode_text = bbcode_text.format(heading_sizes)
	return bbcode_text
