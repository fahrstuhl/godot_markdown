extends Control

onready var cmark = preload("res://bin/cmark.gdns").new()

func _on_Button_pressed():
	var markdown = "literal text *italics text* __fat text__ [hallo](https://welt.what.re)"
	$Label.text = "Data = " + cmark.convert_markdown(markdown)
