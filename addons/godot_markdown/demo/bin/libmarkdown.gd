extends Node
class_name Markdown

static func convert_markdown(text):
	var cmark = Cmark.new()
	var heading_sizes = {
		"font_size=h1": "font_size=%d" % (cmark.font_size * cmark.h1_size),
		"font_size=h2": "font_size=%d" % (cmark.font_size * cmark.h2_size),
		"font_size=h3": "font_size=%d" % (cmark.font_size * cmark.h3_size),
		"font_size=h4": "font_size=%d" % (cmark.font_size * cmark.h4_size),
		"font_size=h5": "font_size=%d" % (cmark.font_size * cmark.h5_size),
		"font_size=h6": "font_size=%d" % (cmark.font_size * cmark.h6_size)
	}
	var bbcode_text = cmark.convert_markdown(text)
	bbcode_text = bbcode_text.format(heading_sizes)
	print(bbcode_text)
	return bbcode_text
