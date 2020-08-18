# Markdown to Richtext Conversion for Godot

Convert from [GitHub Flavored Markdown](https://github.github.com/gfm/) to [Godot Richtext](https://docs.godotengine.org/en/stable/tutorials/gui/bbcode_in_richtextlabel.html).

## Usage

Download the most recent release and extract it into the root of your project.
This creates a `res://bin` and a `res://fonts/libmarkdown` directory.
You can then call the conversion method with `Markdown.convert_markdown(markdown_formatted_string)`.

[RichTextLabels](https://docs.godotengine.org/en/stable/classes/class_richtextlabel.html) need a very verbose font setup right now.
I've included some of the [Google Noto fonts](https://www.google.com/get/noto/) and the project is preconfigured to use them.
If you want to use different fonts, have a look at the demo project for an example.
You need a [DynamicFont](https://docs.godotengine.org/en/3.2/classes/class_dynamicfont.html) for regular, _italics_, __bold__, ___bold italics___ and `monospaced` text.
You also need to set the fonts for the six heading fonts in `res://fonts/libmarkdown/` because you can't change font size in Godot richtext without creating a new font of that size.
